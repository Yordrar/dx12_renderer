#pragma once

template<typename T>
class Manager
{
protected:
    Manager() noexcept = default;
    Manager( const Manager& ) = delete;

    virtual ~Manager() = default;

    Manager& operator=( const Manager& ) = delete;

public:
    static T& it() noexcept( std::is_nothrow_constructible<T>::value )
    {
        static T instance{};
        return instance;
    }
};