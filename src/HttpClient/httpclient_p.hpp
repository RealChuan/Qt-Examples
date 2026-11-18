#pragma once

#include "httpclient.hpp"

#include <QFile>
#include <QHash>
#include <QLoggingCategory>
#include <QNetworkRequest>
#include <QPointer>
#include <QTimer>

#include <chrono>
#include <memory>
#include <optional>
#include <unordered_map>

Q_DECLARE_LOGGING_CATEGORY(lcHttpClient)

namespace Utils {

// 每个在途请求的执行期状态，由 HttpClientPrivate::tasks 以 unique_ptr 持有（RAII）：
// 析构即停止超时定时器、关闭文件句柄
struct TaskState
{
    HttpFinishedCallback onFinished;
    HttpProgressCallback onProgress;
    std::unique_ptr<QFile> file;          // 上传：源文件；下载：目标 .temp
    std::unique_ptr<QTimer> timeoutTimer; // 总时长超时定时器
    QString downloadPath;                 // 仅下载：最终路径
    qint64 downloadBase = 0;              // 仅下载：断点续传偏移
    bool ignoreSsl = false;
    bool rangeChecked = false;            // 仅下载：首个 readyRead 已处理 200 截断分支
    QString fileError;                    // 仅下载：写入/保存失败原因；非空即失败
    // QNetworkReply 无 sslErrors() getter，信号是唯一来源：槽中收集，buildResult 时带入
    QList<QSslError> sslErrors;
    bool timedOut = false; // 超时定时器已触发：buildResult 据此区分 Timeout 与 QNAM 内部取消
    QByteArray ioChunk;    // drainToFile 复用的落盘缓冲：跨多次 readyRead 避免重复分配 1 MiB
    // 请求侧快照：verb/url/headers 由 prepare 记录，body 是副本（config 在 send() 后析构）
    HttpRequestSnapshot request;
};

// prepare 的产出：reply 为空表示请求未发起（文件打开失败等），error 携带原因
struct Prepared
{
    QNetworkReply *reply = nullptr;
    std::unique_ptr<TaskState> state;
    HttpResult error;

    [[nodiscard]] bool ok() const { return reply != nullptr; }

    static Prepared fail(HttpResult err, std::unique_ptr<TaskState> state = nullptr)
    { return {.state = std::move(state), .error = std::move(err)}; }
};

// JSON 序列化为请求体：空对象产出 "{}"（有效 JSON），
// 与 std::nullopt 表达的"无 body"严格区分
struct BodyEncoding
{
    QByteArray data;
    QByteArray contentType;
};

BodyEncoding encodeJsonBody(const QJsonObject &json);

// 敏感头值打码（与 redactedUrl 的 *** 占位约定一致）
QByteArray redactedHeader(QByteArrayView name, const QByteArray &value);

} // namespace Utils

class RequestBuilder::RequestBuilderPrivate
{
public:
    RequestBuilderPrivate() = default;

    QPointer<HttpClient> clientPtr;
    RequestBuilder::Kind kind = RequestBuilder::Kind::Json;
    HttpClient::Method method = HttpClient::Method::Get;

    // body 三态：nullopt = 未设置；有值 = 已设置（空字节即空体；
    // json({}) 序列化为 "{}"，与"无 body"严格区分）
    std::optional<QByteArray> bodyData;

    QByteArray verbOverride; // 字符串动词覆盖；effectiveVerb() 决议

    QUrl url;
    QString downloadPath;

    RequestBuilder::PayloadKind payloadKind = RequestBuilder::PayloadKind::None;
    QString filePath;
    QByteArray data;
    QString fileName;

    QHttpHeaders headers; // 覆盖语义由 replaceOrAppend 提供（同名大小写不敏感替换）
    QList<QPair<QString, QString>> queries;
    std::chrono::milliseconds timeout{0};
    bool ignoreSslErrors = false;
    QList<QSslCertificate> clientCertChain; // mTLS 客户端证书链（空 = 未设置，链首为叶证书）
    QSslKey clientKey;
    qint64 readBufferLimit = 0; // 下载在途窗口覆盖值；0 = 使用默认 kDownloadReadBufferLimit
    bool consumed = false;      // send()/sync() 一次性守卫
    HttpFinishedCallback onFinished;
    HttpProgressCallback onProgress;
};

class HttpClient::HttpClientPrivate
{
public:
    HttpClientPrivate() = default;

    static QByteArray methodBytes(HttpClient::Method method);
    static QByteArray effectiveVerb(const RequestBuilder::RequestBuilderPrivate &config);
    static HttpResult fileError(const QUrl &url, const QString &path, const QString &reason);
    static HttpResult invalidUse(const QUrl &url, const QString &reason);

    // excludeHeader：由调用方独占管理的头（大小写不敏感）。用户经 header()
    // 设置的该头不进入最终请求（如 download() 的 Range），避免双重管理
    QNetworkRequest buildRequest(const RequestBuilder::RequestBuilderPrivate &config,
                                 QByteArrayView excludeHeader = {});

    // 分块排空 reply 并写入下载文件（readAll() 整体分配复制在 32 位下会 OOM）；
    // chunk 由 TaskState 持有、跨多次 readyRead 复用。返回空串表示成功，
    // 非空为写入失败原因（磁盘满等）
    static QString drainToFile(QNetworkReply *reply, QFile *file, QByteArray &chunk);

    static HttpResult buildResult(QNetworkReply *reply, const Utils::TaskState &state);

    // 生命周期契约：tasks 的键是非拥有的 reply 视图（reply 由 deleteLater 释放）。
    // 任何终止路径（onReplyFinished/onDownloadFinished/cancelInternal）必须先 erase
    // 再 reply->deleteLater()——reply 若在仍被键入时析构，地址一旦被新 reply 复用，
    // 陈旧条目会被误命中，TaskState 张冠李戴。cancelInternal 还须先 disconnect 再
    // abort（abort 会同步触发 finished，未断开将重入槽访问已移除的状态）
    std::unordered_map<QNetworkReply *, std::unique_ptr<Utils::TaskState>> tasks;

    // BasicCredentials 嵌套于此而非匿名 namespace：外部链接类的成员使用
    // 内部链接类型会触发 -Wsubobject-linkage（CI 有 -Werror）
    struct BasicCredentials
    {
        QString user;
        QString password;
    };

    QHash<QString, BasicCredentials> hostBasicAuth;
};
