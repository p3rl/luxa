#include <luxa/input/input.h>
#include <luxa/platform_win32.h>

struct lx_input
{
    lx_allocator_t *allocator;
    int32_t mouse_x;
    int32_t mouse_y;
};

lx_input_t *lx_input_create(lx_allocator_t *allocator)
{
    lx_input_t *input = lx_alloc(allocator, sizeof(lx_input_t));

    *input = (lx_input_t) { 0 };
    input->allocator = allocator;
    return input;
}

void lx_input_destroy(lx_input_t *input)
{
    lx_free(input->allocator, input);
}

void lx_input_frame_begin(lx_input_t *input, void *input_message)
{
}

void lx_input_frame_end(lx_input_t *input)
{
}
