#pragma once

#include <LifecycleCallback/lifecyclecallback.hpp>

#include <QAbstractSocket>
#include <QByteArray>

class TcpConnection;

// 连接回调接口定义
struct ConnectionCallbacks
{
    // 数据读取回调 (连接指针, 接收到的数据)
    LifecycleCallback<class TcpConnection *, const QByteArray &> onDataReceived;

    // 错误回调 (连接指针, 错误信息)
    LifecycleCallback<class TcpConnection *, QAbstractSocket::SocketError> onErrorOccurred;

    // 连接建立回调 (连接指针)
    LifecycleCallback<class TcpConnection *> onConnected;

    // 连接断开回调 (连接指针)
    LifecycleCallback<class TcpConnection *> onDisconnected;

    // 默认构造函数 - 提供空回调
    ConnectionCallbacks() = default;

    // 便捷构造函数
    ConnectionCallbacks(
        LifecycleCallback<class TcpConnection *, const QByteArray &> dataCallback,
        LifecycleCallback<class TcpConnection *, QAbstractSocket::SocketError> errorCallback = {},
        LifecycleCallback<class TcpConnection *> connectedCallback = {},
        LifecycleCallback<class TcpConnection *> disconnectedCallback = {})
        : onDataReceived(std::move(dataCallback))
        , onErrorOccurred(std::move(errorCallback))
        , onConnected(std::move(connectedCallback))
        , onDisconnected(std::move(disconnectedCallback))
    {}
};