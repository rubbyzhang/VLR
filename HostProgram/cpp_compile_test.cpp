#include "common.h"
#include <VLR/VLRCpp.h>

// Just for compile test for C++ language.

static VLRCpp::Image2DRef loadImage2D(const VLRCpp::ContextRef& context, const std::string& filepath, const std::string& spectrumType, const std::string& colorSpace) {
    return nullptr;
}

static void Cpp_CompileTest() {
    bool enableLogging = true;
    int32_t renderTargetSizeX = 512;
    int32_t renderTargetSizeY = 512;
    bool firstFrame = true;
    uint32_t numAccumFrames;

    using namespace VLR;
    using namespace VLRCpp;

    ContextRef context = Context::create(enableLogging);

    // Construct a scene by defining meshes and materials.

    SceneRef scene = context->createScene();

    TriangleMeshSurfaceNodeRef mesh = context->createTriangleMeshSurfaceNode("My Mesh 1");
    {
        Vertex vertices[] = {
            Vertex{ Point3D(-1.5f,  0.0f, -1.5f), Normal3D(0,  1, 0), Vector3D(1,  0,  0), TexCoord2D(0.0f, 5.0f) },
            // ...
        };
        // ...
        mesh->setVertices(vertices, lengthof(vertices));

        {
            Image2DRef imgAlbedo = loadImage2D(context, "checkerboard.png", "Reflectance", "Rec709(D65) sRGB Gamma");
            Image2DRef imgNormalAlpha = loadImage2D(context, "normal_alpha.png", "NA", "Rec709(D65)");

            ShaderNodeRef nodeAlbedo = context->createShaderNode("Image2DTexture");
            nodeAlbedo->set("image", imgAlbedo);
            nodeAlbedo->set("min filter", "Nearest");
            nodeAlbedo->set("mag filter", "Nearest");

            ShaderNodeRef nodeNormalAlpha = context->createShaderNode("Image2DTexture");
            nodeNormalAlpha->set("image", imgNormalAlpha);

            // You can flexibly define a material by connecting shader nodes.
            SurfaceMaterialRef mat = context->createSurfaceMaterial("Matte");
            mat->set("albedo", nodeAlbedo->getPlug(VLRShaderNodePlugType_Spectrum, 0));

            ShaderNodeRef nodeTangent = context->createShaderNode("Tangent");
            nodeTangent->set("tangent type", "Radial Y");

            uint32_t matGroup[] = { 0, 1, 2, 0, 2, 3 };
            mesh->addMaterialGroup(matGroup, lengthof(matGroup), mat,
                                   nodeNormalAlpha->getPlug(VLRShaderNodePlugType_Normal3D, 0), // normal map
                                   nodeTangent->getPlug(VLRShaderNodePlugType_Vector3D, 0), // tangent
                                   nodeNormalAlpha->getPlug(VLRShaderNodePlugType_Alpha, 0)); // alpha map
        }

        // ...
    }

    // You can construct a scene graph with transforms
    InternalNodeRef transformNode = context->createInternalNode("trf A");
    transformNode->setTransform(context->createStaticTransform(scale(2.0f)));
    transformNode->addChild(mesh);
    scene->addChild(transformNode);

    // Setup a camera
    CameraRef camera = context->createCamera("Perspective");
    camera->set("position", Point3D(0, 1.5f, 6.0f));
    camera->set("aspect", (float)renderTargetSizeX / renderTargetSizeY);
    camera->set("sensitivity", 1.0f);
    camera->set("fovy", 40 * M_PI / 180);
    camera->set("lens radius", 0.0f);

    // Setup the output buffer (OpenGL buffer can also be attached)
    context->bindOutputBuffer(1024, 1024, 0);

    // Let's render the scene!
    context->render(scene, camera, 1, firstFrame, &numAccumFrames);
}
