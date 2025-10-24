#pragma once

#include <QDebug>
#include <QObject>
#include <QPointer>

#include <functional>
#include <memory>
#include <type_traits>

/**
 * @brief 生命周期感知的回调包装器
 * 支持 QObject、std::shared_ptr 和自由函数，自动处理对象销毁安全
 */
template<typename... Args>
class LifecycleCallback
{
public:
    // 默认构造
    LifecycleCallback() = default;

    // 从任意可调用对象构造（无生命周期保护）
    template<typename F,
             typename = std::enable_if_t<!std::is_same_v<std::decay_t<F>, LifecycleCallback>
                                         && std::is_invocable_v<F, Args...>>>
    LifecycleCallback(F &&f)
        : m_impl(std::make_shared<GenericImpl<F>>(std::forward<F>(f)))
    {}

    // 从 QObject 成员函数构造
    template<typename T, typename = std::enable_if_t<std::is_base_of_v<QObject, T>>>
    LifecycleCallback(T *obj, void (T::*method)(Args...))
        : m_impl(std::make_shared<QObjectMemberImpl<T>>(obj, method))
    {}

    // 从 std::shared_ptr 成员函数构造
    template<typename T>
    LifecycleCallback(std::shared_ptr<T> obj, void (T::*method)(Args...))
        : m_impl(std::make_shared<SharedPtrMemberImpl<T>>(std::move(obj), method))
    {}

    // 执行回调
    void operator()(Args... args) const
    {
        if (m_impl && m_impl->isValid()) {
            m_impl->invoke(std::forward<Args>(args)...);
        } else {
            qWarning() << "LifecycleCallback is invalid.";
        }
    }

    // 检查是否有效
    explicit operator bool() const { return m_impl && m_impl->isValid(); }

    // 检查是否可安全调用
    bool isValid() const { return m_impl && m_impl->isValid(); }

private:
    // 回调实现接口
    struct ImplBase
    {
        virtual ~ImplBase() = default;
        virtual void invoke(Args... args) = 0;
        virtual bool isValid() const = 0;
    };

    // 通用实现（无生命周期保护）
    template<typename F>
    class GenericImpl : public ImplBase
    {
    public:
        GenericImpl(F &&f)
            : m_func(std::forward<F>(f))
        {}

        void invoke(Args... args) override { m_func(std::forward<Args>(args)...); }

        bool isValid() const override
        {
            // 修复：Lambda 和函数对象总是有效的
            // 只有 std::function 需要检查是否为空
            if constexpr (std::is_same_v<std::decay_t<F>, std::function<void(Args...)>>) {
                return static_cast<bool>(m_func);
            } else {
                return true;
            }
        }

    private:
        F m_func;
    };

    // QObject 成员函数实现
    template<typename T>
    class QObjectMemberImpl : public ImplBase
    {
    public:
        QObjectMemberImpl(T *obj, void (T::*method)(Args...))
            : m_context(obj)
            , m_method(method)
        {}

        void invoke(Args... args) override
        {
            if (auto ptr = m_context.data()) {
                (ptr->*m_method)(std::forward<Args>(args)...);
            }
        }

        bool isValid() const override { return !m_context.isNull(); }

    private:
        QPointer<T> m_context;
        void (T::*m_method)(Args...);
    };

    // std::shared_ptr 成员函数实现
    template<typename T>
    class SharedPtrMemberImpl : public ImplBase
    {
    public:
        SharedPtrMemberImpl(std::shared_ptr<T> obj, void (T::*method)(Args...))
            : m_weakObj(std::move(obj))
            , m_method(method)
        {}

        void invoke(Args... args) override
        {
            if (auto ptr = m_weakObj.lock()) {
                (ptr.get()->*m_method)(std::forward<Args>(args)...);
            }
        }

        bool isValid() const override { return !m_weakObj.expired(); }

    private:
        std::weak_ptr<T> m_weakObj;
        void (T::*m_method)(Args...);
    };

    std::shared_ptr<ImplBase> m_impl;
};

// 便捷创建函数
namespace LifecycleCallbackFactory {

// 从 QObject 成员函数创建
template<typename T, typename... Args>
LifecycleCallback<Args...> create(T *obj, void (T::*method)(Args...))
{
    return LifecycleCallback<Args...>(obj, method);
}

// 从 std::shared_ptr 成员函数创建
template<typename T, typename... Args>
LifecycleCallback<Args...> create(std::shared_ptr<T> obj, void (T::*method)(Args...))
{
    return LifecycleCallback<Args...>(std::move(obj), method);
}

} // namespace LifecycleCallbackFactory

// 简化创建函数
template<typename T, typename... Args>
LifecycleCallback<Args...> makeLifecycleCallback(T *obj, void (T::*method)(Args...))
{
    return LifecycleCallback<Args...>(obj, method);
}

template<typename T, typename... Args>
LifecycleCallback<Args...> makeLifecycleCallback(std::shared_ptr<T> obj, void (T::*method)(Args...))
{
    return LifecycleCallback<Args...>(std::move(obj), method);
}

template<typename F, typename... Args>
LifecycleCallback<Args...> makeLifecycleCallback(F &&f)
{
    return LifecycleCallback<Args...>(std::forward<F>(f));
}
