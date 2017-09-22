#include <test/luxa/renderer/scene_tests.h>
#include <luxa/test.h>
#include <luxa/renderer/scene.h>

void create_scene_succeeds()
{
    // Arrange
    lx_allocator_t *allocator = lx_allocator_default();

    // Act
    lx_scene_t *scene = lx_scene_create(allocator);

    // Assert
    LX_ASSERT(scene, "Invalid scene");
    lx_scene_destroy(scene);
}

void create_hierarchy_succeeds()
{
    // Arrange
    lx_allocator_t *allocator = lx_allocator_default();
    lx_scene_t *scene = lx_scene_create(allocator);

    // Act
    lx_scene_node_t root = lx_scene_root_node();
    lx_scene_node_t a = lx_scene_create_node(scene, root);
    lx_scene_node_t b = lx_scene_create_node(scene, root);
    lx_scene_node_t c = lx_scene_create_node(scene, root);

    lx_scene_node_t a1 = lx_scene_create_node(scene, a);
    lx_scene_node_t b1 = lx_scene_create_node(scene, b);
    lx_scene_node_t c1 = lx_scene_create_node(scene, c);

    // Assert
    lx_scene_node_t child = lx_scene_first_child(scene, lx_scene_root_node());
    LX_EQUALS(child, 2);
    child = lx_scene_next_sibling(scene, child);
    LX_EQUALS(child, 3);
    child = lx_scene_next_sibling(scene, child);
    LX_EQUALS(child, 4);

    child = lx_scene_first_child(scene, a);
    LX_EQUALS(child, 5);
    child = lx_scene_first_child(scene, b);
    LX_EQUALS(child, 6);
    child = lx_scene_first_child(scene, c);
    LX_EQUALS(child, 7);
}

void setup_scene_test_fixture()
{
    LX_TEST_FIXTURE_BEGIN("Scene")
        LX_ADD_TEST(create_scene_succeeds);
        LX_ADD_TEST(create_hierarchy_succeeds);
    LX_TEST_FIXTURE_END()
}