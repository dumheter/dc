#include <dc/dtest.hpp>
#include <dc/macros.hpp>
#include <dc/traits.hpp>
#include <type_traits>

using namespace dc;

struct DummyStruct {};
int dummyFunction(int, float, double, DummyStruct);
template <typename... Args>
void dummyVariadicFunction(int, Args... args);

struct NoMove {
  DC_DELETE_MOVE(NoMove);
};

DTEST(isReference) {
  static_assert(isReference<int&>);
  static_assert(!isReference<int>);

  ASSERT_TRUE(true);
}

DTEST(isVoid) {
  static_assert(isVoid<void>);
  static_assert(!isVoid<int>);

  ASSERT_TRUE(true);
}

DTEST(isLValueReference) {
  static_assert(isLValueReference<int&>);
  static_assert(!isLValueReference<int>);

  ASSERT_TRUE(true);
}

DTEST(isFunction) {
  static_assert(isFunction<int(int)>);
  static_assert(isFunction<decltype(dummyFunction)>);
  static_assert(
      isFunction<decltype(dummyVariadicFunction<float, DummyStruct>)>);
  static_assert(!isFunction<int>);

  ASSERT_TRUE(true);
}

DTEST(_move) {
  // TODO cgustafsson:
  ASSERT_TRUE(true);
}

DTEST(_swap) {
  // TODO cgustafsson:
  ASSERT_TRUE(true);
}

DTEST(isRValueReference) {
  static_assert(isRValueReference<int&&>);
  static_assert(!isRValueReference<int>);
  static_assert(!isRValueReference<int&>);

  ASSERT_TRUE(true);
}

DTEST(isObject) {
  static_assert(!isObject<void>);
  static_assert(!isObject<int&>);
  static_assert(!isObject<int&>);
  static_assert(isObject<DummyStruct>);
  static_assert(isObject<int>);
  static_assert(!isObject<decltype(dummyFunction)>);

  ASSERT_TRUE(true);
}

DTEST(isConstructible) {
  static_assert(!isConstructible<void>);
  static_assert(!isConstructible<int&>);
  static_assert(!isConstructible<int&>);
  static_assert(isConstructible<DummyStruct>);
  static_assert(isConstructible<int>);
  static_assert(!isConstructible<decltype(dummyFunction)>);

  ASSERT_TRUE(true);
}

DTEST(isMoveConstructible) {
  static_assert(!isMoveConstructible<NoMove>);
  static_assert(isMoveConstructible<DummyStruct>);
  static_assert(isMoveConstructible<int>);
  static_assert(!isMoveConstructible<decltype(dummyFunction)>);

  ASSERT_TRUE(true);
}

DTEST(YesNoType) {
  static_assert(sizeof(YesType) != sizeof(NoType));

  ASSERT_TRUE(true);
}

DTEST(addLValueReference) {
  static_assert(isSame<int&, AddLValueReference<int>::Type>);
  static_assert(isSame<int&, AddLValueReference<int&>::Type>);
  static_assert(isSame<int&, AddLValueReference<int&&>::Type>);

  ASSERT_TRUE(true);
}

DTEST(isAssignable) {
  static_assert(!isAssignable<NoMove, NoMove>);
  static_assert(isAssignable<DummyStruct, DummyStruct>);
  // TODO cgustafsson: broken? \/
  // static_assert(isAssignable<int, int>);
  static_assert(
      !isAssignable<decltype(dummyFunction), decltype(dummyFunction)>);

  ASSERT_TRUE(true);
}

DTEST(isSame) {
  static_assert(isSame<int, int>);
  static_assert(!isSame<int, short>);
  static_assert(isSame<DummyStruct, DummyStruct>);

  ASSERT_TRUE(true);
}

DTEST(isSwappable) {
  static_assert(isSwappable<int>);
  static_assert(isSwappable<short>);
  static_assert(isSwappable<DummyStruct>);
  // TODO cgustafsson: broken? \/
  // static_assert(!isSwappable<NoMove>);
  // static_assert(!isSwappable<decltype(dummyFunction)>);

  ASSERT_TRUE(true);
}

DTEST(isMovable) {
  static_assert(isMovable<int>);
  static_assert(!isMovable<NoMove>);
  static_assert(isMovable<DummyStruct>);

  ASSERT_TRUE(true);
}

DTEST(isConst) {
  static_assert(!isConst<int>);
  static_assert(!isConst<NoMove>);
  static_assert(!isConst<DummyStruct>);

  static_assert(isConst<const int>);
  static_assert(isConst<const NoMove>);
  static_assert(isConst<const DummyStruct>);

  ASSERT_TRUE(true);
}

DTEST(addConst) {
  static_assert(isSame<const int, AddConst<int>::Type>);
  static_assert(isConst<AddConst<DummyStruct>::Type>);

  static_assert(isSame<const int, AddConst<const int>::Type>);
  static_assert(isConst<AddConst<const DummyStruct>::Type>);

  ASSERT_TRUE(true);
}

DTEST(removeConst) {
  static_assert(isSame<int, RemoveConst<const int>::Type>);
  static_assert(!isConst<RemoveConst<const DummyStruct>::Type>);

  static_assert(isSame<int, RemoveConst<int>::Type>);
  static_assert(!isConst<RemoveConst<DummyStruct>::Type>);

  ASSERT_TRUE(true);
}

DTEST(isIterable) {
  // TODO cgustafsson:
  ASSERT_TRUE(true);
}

DTEST(isArray) {
  static_assert(!isArray<int>);
  static_assert(isArray<int[]>);
  static_assert(isArray<const int[]>);
  static_assert(isArray<const int[1337]>);

  ASSERT_TRUE(true);
}

DTEST(removeExtent) {
  static_assert(isSame<int, RemoveExtent<int>::Type>);
  static_assert(isSame<int, RemoveExtent<int[]>::Type>);
  static_assert(isSame<const int, RemoveExtent<const int[]>::Type>);

  ASSERT_TRUE(true);
}

DTEST(addPointer) {
  static_assert(isSame<int*, AddPointer<int>::Type>);
  static_assert(!isSame<int, AddPointer<int>::Type>);

  ASSERT_TRUE(true);
}

DTEST(removeVolatile) {
  static_assert(isSame<int, RemoveVolatile<volatile int>::Type>);
  static_assert(isSame<int, RemoveVolatile<int>::Type>);
  static_assert(!isSame<volatile int, RemoveVolatile<volatile int>::Type>);

  ASSERT_TRUE(true);
}

DTEST(removeCV) {
  static_assert(isSame<int, RemoveCV<const volatile int>::Type>);
  static_assert(isSame<int, RemoveCV<volatile int>::Type>);
  static_assert(isSame<int, RemoveCV<const int>::Type>);
  static_assert(isSame<int, RemoveCV<int>::Type>);
  static_assert(
      !isSame<const volatile int, RemoveCV<const volatile int>::Type>);

  ASSERT_TRUE(true);
}

DTEST(conditional) {
  static_assert(isSame<double, Conditional<(sizeof(double) > sizeof(float)),
                                           double, float>::Type>);
  static_assert(isSame<float, Conditional<(sizeof(double) < sizeof(float)),
                                          double, float>::Type>);

  ASSERT_TRUE(true);
}

DTEST(decay) {
  static_assert(isSame<int, Decay<int>::Type>);
  static_assert(isSame<int, Decay<int&>::Type>);
  static_assert(isSame<int, Decay<int&&>::Type>);
  static_assert(!isSame<int, Decay<float&>::Type>);
  static_assert(isSame<int*, Decay<int[]>::Type>);

  ASSERT_TRUE(true);
}

DTEST(enableIf) {
  // NOTE: dont really need to test here, its used all over
  ASSERT_TRUE(true);
}

DTEST(isBaseOf) {
  struct A {};
  struct B : A {};

  static_assert(isBaseOf<A, B>);
  static_assert(isBaseOf<A, A>);
  static_assert(!isBaseOf<B, A>);

  ASSERT_TRUE(true);
}

DTEST(isEqualityComparable) {
  struct A {
    bool operator==(const A& other);
    bool operator!=(const A& other);
  };

  static_assert(isEqualityComparable<A, A>);
  static_assert(isEqualityComparable<int, int>);
  static_assert(!isEqualityComparable<A, int>);

  ASSERT_TRUE(true);
}

DTEST(isRef) {
  static_assert(isRef<Ref<int>>);
  static_assert(!isRef<int>);

  ASSERT_TRUE(true);
}

DTEST(InvokeResult) {
  const auto fn = [](int, float) -> short { return 0; };
  static_assert(isSame<short, InvokeResultT<decltype(fn), int, float>>);

  ASSERT_TRUE(true);
}

DTEST(isInvocable) {
  const auto fn = [](int) {};
  static_assert(isInvocable<decltype(fn), int>);

  ASSERT_TRUE(true);
}

DTEST(isPod) {
  struct Abc {};
  static_assert(isPod<Abc>);

  static_assert(isPod<float>);

  struct Complex {
    Complex(int, float) {}
  };
  static_assert(!isPod<Complex>);

  ASSERT_TRUE(true);
}

DTEST(isFloatingPoint) {
  static_assert(isFloatingPoint<f32>);
  static_assert(isFloatingPoint<f64>);
  static_assert(!isFloatingPoint<s32>);

  ASSERT_TRUE(true);
}

DTEST(isIntegral) {
  static_assert(isIntegral<bool>);
  static_assert(isIntegral<u8>);
  static_assert(isIntegral<s8>);
  static_assert(isIntegral<u16>);
  static_assert(isIntegral<s16>);
  static_assert(isIntegral<u32>);
  static_assert(isIntegral<s32>);
  static_assert(isIntegral<u64>);
  static_assert(isIntegral<s64>);
  static_assert(isIntegral<char8>);

  static_assert(!isIntegral<f32>);

  ASSERT_TRUE(true);
}

DTEST(isArithmetic) {
  static_assert(isArithmetic<bool>);
  static_assert(isArithmetic<u8>);
  static_assert(isArithmetic<s8>);
  static_assert(isArithmetic<u16>);
  static_assert(isArithmetic<s16>);
  static_assert(isArithmetic<u32>);
  static_assert(isArithmetic<s32>);
  static_assert(isArithmetic<u64>);
  static_assert(isArithmetic<s64>);
  static_assert(isArithmetic<char8>);

  static_assert(isArithmetic<f32>);
  static_assert(isArithmetic<f64>);

  static_assert(!isArithmetic<DummyStruct>);

  ASSERT_TRUE(true);
}

DTEST(isFundamental) {
  static_assert(isFundamental<bool>);
  static_assert(isFundamental<u8>);
  static_assert(isFundamental<s8>);
  static_assert(isFundamental<u16>);
  static_assert(isFundamental<s16>);
  static_assert(isFundamental<u32>);
  static_assert(isFundamental<s32>);
  static_assert(isFundamental<u64>);
  static_assert(isFundamental<s64>);
  static_assert(isFundamental<char8>);

  static_assert(isFundamental<f32>);
  static_assert(isFundamental<f64>);

  static_assert(isFundamental<void>);

  static_assert(!isFundamental<DummyStruct>);

  ASSERT_TRUE(true);
}

DTEST(isTriviallyRelocatable) {
  static_assert(isTriviallyRelocatable<bool>);
  static_assert(isTriviallyRelocatable<u8>);
  static_assert(isTriviallyRelocatable<s8>);
  static_assert(isTriviallyRelocatable<u16>);
  static_assert(isTriviallyRelocatable<s16>);
  static_assert(isTriviallyRelocatable<u32>);
  static_assert(isTriviallyRelocatable<s32>);
  static_assert(isTriviallyRelocatable<u64>);
  static_assert(isTriviallyRelocatable<s64>);
  static_assert(isTriviallyRelocatable<char8>);

  static_assert(isTriviallyRelocatable<f32>);
  static_assert(isTriviallyRelocatable<f64>);

  static_assert(isTriviallyRelocatable<void>);

  static_assert(!isTriviallyRelocatable<DummyStruct>);

  struct Rel {
    using IsTriviallyRelocatable = bool;
  };
  static_assert(isTriviallyRelocatable<Rel>);

  ASSERT_TRUE(true);
}
