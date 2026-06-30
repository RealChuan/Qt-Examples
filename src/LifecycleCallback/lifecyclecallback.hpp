#pragma once

#include <concepts>
#include <functional>
#include <memory>
#include <type_traits>
#include <utility>

/**
 * @brief 生命周期安全的回调包装器
 *
 * 支持任意可调用对象以及 std::shared_ptr 成员函数。
 */
template<typename... Args>
class LifecycleCallback
{
public:
    LifecycleCallback() = default;

    template<typename F>
        requires(!std::same_as<std::decay_t<F>, LifecycleCallback> && std::invocable<F, Args...>)
    LifecycleCallback(F &&f)
        : m_impl(std::make_shared<GenericImpl<std::decay_t<F>>>(std::forward<F>(f)))
    {}

    template<typename T>
    LifecycleCallback(std::shared_ptr<T> obj, void (T::*method)(Args...))
        : m_impl(std::make_shared<SharedPtrMemberImpl<T>>(std::move(obj), method))
    {}

    /**
     * @brief 扩展点：由其他头文件（如 lifecyclecallback_qt.hpp）传入带生命周期检查的调用器。
     */
    LifecycleCallback(std::function<void(Args...)> invoker, std::function<bool()> validator)
        : m_impl(std::make_shared<TrackedImpl>(std::move(invoker), std::move(validator)))
    {}

    void operator()(Args... args) const
    {
        if (m_impl) [[likely]] {
            m_impl->invoke(std::forward<Args>(args)...);
        }
    }

    [[nodiscard]] explicit operator bool() const { return m_impl && m_impl->isValid(); }

    [[nodiscard]] bool isValid() const { return m_impl && m_impl->isValid(); }

private:
    struct ImplBase
    {
        virtual ~ImplBase() = default;
        virtual void invoke(Args... args) = 0;
        virtual bool isValid() const = 0;
    };

    template<typename F>
    class GenericImpl : public ImplBase
    {
    public:
        explicit GenericImpl(F f) : m_func(std::move(f)) {}

        void invoke(Args... args) override
        {
            if constexpr (std::is_same_v<F, std::function<void(Args...)>>) {
                if (!m_func) {
                    return;
                }
            } else if constexpr (std::is_pointer_v<F>
                                 && std::is_function_v<std::remove_pointer_t<F>>) {
                if (!m_func) {
                    return;
                }
            }

            m_func(std::forward<Args>(args)...);
        }

        bool isValid() const override
        {
            if constexpr (std::is_same_v<F, std::function<void(Args...)>>) {
                return static_cast<bool>(m_func);
            }
            if constexpr (std::is_pointer_v<F> && std::is_function_v<std::remove_pointer_t<F>>) {
                return m_func != nullptr;
            }
            return true;
        }

    private:
        F m_func;
    };

    template<typename T>
    class SharedPtrMemberImpl : public ImplBase
    {
    public:
        SharedPtrMemberImpl(std::shared_ptr<T> obj, void (T::*method)(Args...))
            : m_weakObj(std::move(obj)), m_method(method)
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

    class TrackedImpl : public ImplBase
    {
    public:
        TrackedImpl(std::function<void(Args...)> invoker, std::function<bool()> validator)
            : m_invoker(std::move(invoker)), m_validator(std::move(validator))
        {}

        void invoke(Args... args) override
        {
            if (!m_validator || !m_validator()) {
                return;
            }
            if (m_invoker) {
                m_invoker(std::forward<Args>(args)...);
            }
        }

        bool isValid() const override { return m_validator && m_validator(); }

    private:
        std::function<void(Args...)> m_invoker;
        std::function<bool()> m_validator;
    };

    std::shared_ptr<ImplBase> m_impl;
};

template<typename T, typename... Args>
[[nodiscard]] LifecycleCallback<Args...> makeLifecycleCallback(std::shared_ptr<T> obj,
                                                               void (T::*method)(Args...))
{ return LifecycleCallback<Args...>(std::move(obj), method); }
