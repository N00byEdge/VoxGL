#pragma once

constexpr float lerp(float a, float b, float alpha) {
	return a + alpha * (b - a);
}

constexpr float blerp(float f00, float f01, float f10, float f11, float ax, float ay) {
	return f00 * (1.f - ax) * (1.f - ay) + f10 * ax * (1.f - ay) + f01 * (1.f - ax) * ay + f11 * ax * ay;
}
