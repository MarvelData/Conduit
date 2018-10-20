#include "Math.hpp"

int Math::GreatestCommonDivisor(int a, int b) { return b ? GreatestCommonDivisor(b, a % b) : a; }

int Math::LeastCommonMultiplier(int a, int b) { return a / GreatestCommonDivisor(a, b) * b; }