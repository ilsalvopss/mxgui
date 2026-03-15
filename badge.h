//
// Created by Salvo Passaro on 03/03/26.
// This file is part of mxgui
// and is licensed as the rest of this project.
//

#pragma once

#ifdef MXGUI_LEVEL_2

// inspired by https://awesomekling.github.io/Serenity-C++-patterns-The-Badge/

template<typename T>
class Badge {
    friend T;
    constexpr Badge() noexcept {}

public:
    Badge(const Badge&) = delete;
    Badge& operator=(const Badge&) = delete;
    Badge(Badge&&) = delete;
    Badge& operator=(Badge&&) = delete;
};

#endif //MXGUI_LEVEL_2