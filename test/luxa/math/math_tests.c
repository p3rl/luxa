#include <test/luxa/math/math_tests.h>
#include <luxa/math/math.h>
#include <luxa/test.h>

void vec3_tests()
{
	lx_vec3_t a = { 1.0f, 2.0f, 3.0f };
	lx_vec3_t b = { 2.0f, 3.0f, 4.0f };
	lx_vec3_t result;

	lx_vec3_add(&a, &b, &result);
	LX_TRUE(lx_vec3_near_equal(&result, &(lx_vec3_t) { 3.0f, 5.0f, 7.0f }));

	lx_vec3_sub(&b, &a, &result);
	LX_TRUE(lx_vec3_near_equal(&result, &(lx_vec3_t) { 1.0f, 1.0f, 1.0f }));

	lx_vec3_cross(&(lx_vec3_t) { 1.0f, 0.0f, 0.0f }, &(lx_vec3_t) { 0.0f, 1.0f, 0.0f }, &result);
	LX_TRUE(lx_vec3_near_equal(&result, &(lx_vec3_t) { 0.0f, 0.0f, 1.0f }));

	float dot = lx_vec3_dot(&(lx_vec3_t) { 1.0f, 0.0f, 0.0f }, &(lx_vec3_t) { 0.0f, 1.0f, 0.0f });
	LX_TRUE(lx_near_equalf(0, dot));
}

void matrix_look_at()
{
	lx_vec3_t camera_position = { 0.0f, 0.0f, -5.0f };
	lx_vec3_t camera_target = { 0.0f, 0.0f, 0.0f };
	lx_vec3_t camera_up = { 0.0f, 1.0f, 0.0f };
	
	lx_mat4_t view;
	lx_mat4_look_at(&camera_target, &camera_position, &camera_up, &view);

	lx_mat4_t t, r, v;
	lx_mat4_translation(0.0f, 0.0f, 5.0f, &t);
	lx_mat4_identity(&r);
	lx_mat4_mul(&t, &r, &v);
}

void setup_math_test_fixture()
{
	LX_TEST_FIXTURE_BEGIN("Math")
		LX_ADD_TEST(vec3_tests);
		LX_ADD_TEST(matrix_look_at);
	LX_TEST_FIXTURE_END()
}