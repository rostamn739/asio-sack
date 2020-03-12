#ifndef ASIOSACK_SACK_H
#define ASIOSACK_SACK_H

#include <type_traits>
#include <functional>
#include <utility>

namespace sack::common { // NOLINT(modernize-concat-nested-namespaces)
namespace impl {

template<typename TFunc>
class MoveFunc : public std::function<TFunc> {
    using base_type = std::function<TFunc>;

    template <typename TFuncIn, typename TEnable>
    struct Wrapper { };

    template <typename TFuncIn>
    struct Wrapper<TFuncIn, std::enable_if_t<std::is_copy_constructible<TFuncIn>::value>> {
        TFuncIn func_;

        template <typename... TArgs>
        decltype(auto) operator() (TArgs&&... args) {
            return func_(std::forward<TArgs>(args)...);
        }
    };

    template <typename TFuncIn>
    struct Wrapper<TFuncIn,
            std::enable_if_t<
            std::is_move_constructible<TFuncIn>::value &&
            !std::is_copy_constructible<TFuncIn>::value>
            > {
        TFuncIn func_;

        Wrapper(TFuncIn&& func): func_{std::forward<decltype(func)>(func)}  { // NOLINT(google-explicit-constructor,hicpp-explicit-conversions)

        }

        Wrapper(Wrapper&&) noexcept = default;
        Wrapper& operator= (Wrapper&&) noexcept = default;

        // never called, defined to provide init for non-DefaultConstructible TFuncIn
        Wrapper(const Wrapper& other): func_(const_cast<TFuncIn&&>(other.func_)) {
            ::std::abort();
        }
        Wrapper& operator= (const Wrapper&) { ::std::abort(); }; // NOLINT(cert-oop54-cpp)

        template <typename... TArgs>
        decltype(auto) operator() (TArgs&&... args) {
            return func_(std::forward<TArgs>(args)...);
        }
    };
public:
    using base_type::operator();

    MoveFunc() = default;
    explicit MoveFunc(std::nullptr_t): base_type(nullptr) { };

    MoveFunc(MoveFunc&&) noexcept = default;
    MoveFunc &operator= (MoveFunc&&) noexcept = default;

    MoveFunc &operator= (std::nullptr_t) {
        base_type::operator=(nullptr);
        return *this;
    }

    template <typename TFuncIn>
    explicit MoveFunc(TFuncIn&& func): base_type(Wrapper(std::forward<TFuncIn>(func))) {

    }
    template <typename TFuncIn>
    MoveFunc &operator= (TFuncIn&& func) {
        base_type::operator=(Wrapper(std::forward<TFuncIn>(func)));
        return *this;
    }
};
}
}

#endif //ASIOSACK_SACK_H
