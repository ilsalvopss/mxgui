//
// Created by Salvo Passaro on 03/03/26.
// This file is part of mxgui
// and is licensed as the rest of this project.
//

#pragma once

#ifdef MXGUI_LEVEL_2

#include <type_traits>

// inspired by https://awesomekling.github.io/Serenity-C++-patterns-The-Badge/

/**
 * A Badge<T> is an object that can only be created by T itself.
 * As such, it guarantees that whoever called a method accepting a Badge<T> as parameter is an instance of T.
 */
template<typename T>
class Badge {
protected:
    friend T;
    constexpr Badge() noexcept = default;

public:
    Badge(const Badge&) = delete;
    Badge& operator=(const Badge&) = delete;
    Badge(Badge&&) = delete;
    Badge& operator=(Badge&&) = delete;
};

/**
 * A BadgedRef<T> is a reference to an object of type T that can only be created by T itself.
 * As such, it guarantees that whoever called a method accepting a BadgedRef<T> as parameter is an instance of T.
 */
template<typename T>
class BadgedRef {
    T& ref;

    template<typename>
    friend class BadgedRef;

protected:
    friend T;
    constexpr explicit BadgedRef(T& ref) noexcept : ref(ref) {}

public:
    constexpr T& get() const noexcept { return ref; }
    constexpr explicit operator T&() const noexcept { return ref; }

    template<typename U>
    requires std::is_convertible_v<U*, T*>
    constexpr BadgedRef(const BadgedRef<U>& other) noexcept
        : ref(other.ref) {}

    BadgedRef(const BadgedRef&) = delete;
    BadgedRef& operator=(const BadgedRef&) = delete;
    BadgedRef(BadgedRef&&) = default;
    BadgedRef& operator=(BadgedRef&&) = delete;
};

#endif //MXGUI_LEVEL_2