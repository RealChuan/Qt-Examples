#pragma once

#include "lifecyclecallback.hpp"

#include <QObject>
#include <QPointer>

#include <concepts>
#include <memory>
#include <utility>

/**
 * @brief 为 LifecycleCallback 提供 QObject 生命周期支持。
 */
template<typename T, typename... Args>
    requires std::derived_from<T, QObject>
[[nodiscard]] LifecycleCallback<Args...> makeLifecycleCallback(T *obj, void (T::*method)(Args...))
{
    return LifecycleCallback<Args...>(
        [context = QPointer<T>(obj), method](Args... args) {
            if (auto *ptr = context.data()) {
                (ptr->*method)(std::forward<Args>(args)...);
            }
        },
        [context = QPointer<T>(obj)] { return !context.isNull(); });
}
