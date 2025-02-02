#include <iostream>
#include <chrono>
#include <cmath>
#include <string>
#include <array>
#include <thread>
#include <filesystem>
#include "matrix.h"
#include "window.h"
#include "renderer.h"
#include "timeprofiler.h"


using namespace std::chrono;

using namespace RenderingFramework3D;
using namespace MathUtil;

static int renderer_test();
static std::string get_bin_dir();

int main() {
    return renderer_test();
}

void random_init() {
    srand(time(0));
}

float RandomFloat(float max, float min) {
    return ((max - min) * rand()) / (float)(RAND_MAX)+min;
}

struct LightProperties {
    Vec<4> colour;
    float Intensity = 1;
    float Radius = 1;
};

struct LightInfo {
    Vec<4> position;
    LightProperties properties;
};

static int renderer_test() {
    unsigned windowWidth=1000, windowHeight=800;

    random_init();

//  Create Window
    Window wnd;
    if(wnd.Initialize(false, windowWidth, windowHeight , "Custom Pipeline Test") == false) {
        printf("failed to initialize window\n");
        return -1;
    }

//  Create Renderer
    Renderer renderer;
    if(renderer.Initialize(wnd) == false) {
        printf("failed to initialize renderer\n");
        return -1;
    }

//  Create Camera
    Camera mainCamera({ 0,0,windowWidth, windowHeight });
    mainCamera.SetPosition(Vec<3>({0,-195,42}));
    mainCamera.Rotate(mainCamera.GetCameraAxisX(), -PI/4);

//  Create custom pipeline for rendering the spherical light in the scene
    PipelineConfig lightplcfg;
	lightplcfg.useDefaultShaders = false;
	lightplcfg.useDefaultVertData = false;
    lightplcfg.customVertexShaderPath = get_bin_dir() + "/../../../test/custompipelinetest/shaders/lightvert.spv";
    lightplcfg.customFragmentShaderPath = get_bin_dir() + "/../../../test/custompipelinetest/shaders/lightfrag.spv";
    lightplcfg.uniformShaderInputLayout.ObjectInputs.useMaterialData = false;
    lightplcfg.uniformShaderInputLayout.ObjectInputs.CustomUniformShaderInput.push_back({false, true, sizeof(LightProperties), 1});
    lightplcfg.uniformShaderInputLayout.GlobalInputs.useDirectionalLight = false;
    lightplcfg.uniformShaderInputLayout.ObjectInputs.useObjToWorldTransform = true;
    lightplcfg.uniformShaderInputLayout.ObjectInputs.useWorldToCamTransform = true;
    
    unsigned lightPipeline=(unsigned)-1;
    if(renderer.CreateCustomPipeline(lightplcfg,lightPipeline) == false) {
        printf("failed to create pipeline\n");
        return -1;
    } 

//  Created custom pipeline for objects being lit by the spherical light
    PipelineConfig objplcfg;
	objplcfg.useDefaultShaders = false;
	objplcfg.useDefaultVertData = false;
    objplcfg.customVertexShaderPath = get_bin_dir() + "/../../../test/custompipelinetest/shaders/objvert.spv";
    objplcfg.customFragmentShaderPath = get_bin_dir() + "/../../../test/custompipelinetest/shaders/objfrag.spv";
    objplcfg.uniformShaderInputLayout.GlobalInputs.CustomUniformShaderInput.push_back({false, true, sizeof(LightInfo), 1});
    
    unsigned objPipeline=(unsigned)-1;
    if(renderer.CreateCustomPipeline(objplcfg,objPipeline) == false) {
        printf("failed to create pipeline\n");
        return -1;
    } 

//  Set default global uniform shader inputs including directional and ambient lights
    renderer.SetLightDirection(objPipeline, Vec<3>({-1,-1,0}));
    renderer.SetLightColour(objPipeline, Vec<4>({1,1,1,1}));
    renderer.SetLightIntensity(objPipeline, 0.8);
    renderer.SetAmbientLightIntensity(objPipeline, 0.01);

//  Create Mesh for objects in the scene 
    Mesh spherelightMesh = Mesh::Icosphere(renderer,2);
    Mesh sphereMesh = Mesh::Icosphere(renderer,4);
    Mesh cubeMesh = Mesh::Cube(renderer);

//  Load mesh to GPU memory
    spherelightMesh.LoadMesh();
    sphereMesh.LoadMesh();
    cubeMesh.LoadMesh();

//  Create world objects using the mesh
    WorldObject spherelight(spherelightMesh); 
    WorldObject cube(cubeMesh); 
    WorldObject sphere(sphereMesh); 

//  Set initial attributes for these objects (position, scale, material, etc.)
    cube.Move(Vec<3>({100,-100,150}));
    cube.GetMaterial().colour = Vec<4>({0.8,0.6,0.4,1});
    cube.SetScale(40, 50, 80);

    spherelight.SetScale(50, 50, 50);
    spherelight.SetPosition(Vec<3>({0,0,200}));
    
    sphere.SetScale(50, 50, 50);
    sphere.GetMaterial().colour = Vec<4>({0.2,0.6,0.7,1});
    sphere.SetPosition(Vec<3>({-100,100,300}));

//  Custom data describing spherical light, used as uniform input for objects and light pipelines 
    LightProperties lightProperties = { Vec<4>({0.2,0.5,0.3,1}), 9.500000,7.399995};
    auto position = spherelight.GetPosition();
    LightInfo lightInfo = {Vec<4>({position(0),position(1),position(2),1}), lightProperties};
    renderer.SetCustomGlobalUniformShaderData(objPipeline, 1, &lightInfo, sizeof(lightInfo), 0);
    spherelight.SetCustomUniformShaderInputData(1, &lightProperties, sizeof(lightProperties), 0);
    
    wnd.SetMouseVisibility(false);

//  main loop
    unsigned n = 1000;
    TimeProfiler profiler;
    profiler.Start();
    const float maxCamRotXCos = std::cos(PI/2);
    for (int i = 0;; i++) {
    //  handle FPV camera rotation and movement
        auto disp = wnd.GetMouseDisplacement();

        if (wnd.IsResized()) {
            mainCamera.SetViewPort({ 0,0,wnd.GetWidth(), wnd.GetHeight() });
        }

        float scale = 0.01;
        if (wnd.CheckKeyPressEvent(Window::KEY_LCTRL)) {
            wnd.SetMouseVisibility(true);
        }
        if (wnd.CheckKeyReleaseEvent(Window::KEY_LCTRL)) {
            wnd.SetMouseVisibility(false);
        }

        if (wnd.IsKeyPressed(Window::KEY_LCTRL)==false && wnd.CheckKeyReleaseEvent(Window::KEY_LCTRL)==false && i > 5) {
            if(fabs(disp(0)) > 0.001) {
                mainCamera.Rotate(Vec<3>({0,1,0}), disp(0) * scale);
            }
            if(fabs(disp(1)) > 0.001) {
                mainCamera.Rotate(mainCamera.GetCameraAxisX(), disp(1) * scale);
                if(mainCamera.GetCameraAxisY().Dot(Vec<3>({0,1,0})) < maxCamRotXCos) {
                    mainCamera.Rotate(mainCamera.GetCameraAxisX(), -disp(1) * scale);
                }
            }
        }

        scale = 1.5;
        if (wnd.IsKeyPressed(Window::KEY_W)) {
            mainCamera.Move(scale * mainCamera.GetCameraAxisZ());
        }
        if (wnd.IsKeyPressed(Window::KEY_S)) {
            mainCamera.Move(-scale * mainCamera.GetCameraAxisZ());
        }
        if (wnd.IsKeyPressed(Window::KEY_A)) {
            mainCamera.Move(-scale * mainCamera.GetCameraAxisX());
        }
        if (wnd.IsKeyPressed(Window::KEY_D)) {
            mainCamera.Move(scale * mainCamera.GetCameraAxisX());
        }
        if (wnd.IsKeyPressed(Window::KEY_SPACE)) {
            mainCamera.Move(scale * mainCamera.GetCameraAxisY());
        }
        if (wnd.IsKeyPressed(Window::KEY_LSHIFT)) {
            mainCamera.Move(-scale * mainCamera.GetCameraAxisY() );
        }

    //  Handle user input to move Cube object in the scene
        if (wnd.IsKeyPressed(Window::KEY_I)) {
            cube.Move(Vec<3>({0,scale,0}));
        }
        if (wnd.IsKeyPressed(Window::KEY_K)) {
            cube.Move(Vec<3>({0,-scale,0}));
        }
        if (wnd.IsKeyPressed(Window::KEY_J)) {
            cube.Move(Vec<3>({-scale ,0,0}));
        }
        if (wnd.IsKeyPressed(Window::KEY_L)) {
            cube.Move(Vec<3>({scale ,0,0}));
        }

    //  Handle user input to control spherical light intensity
        float increment = 0.1;
        if(wnd.IsKeyPressed(Window::KEY_T)) {
            lightProperties.Intensity+=increment;
            printf("I %f\n", lightProperties.Intensity);
        } else if(wnd.IsKeyPressed(Window::KEY_G)) {
            lightProperties.Intensity-=increment;
            if(lightProperties.Intensity < increment) lightProperties.Intensity = increment;
            printf("I %f\n", lightProperties.Intensity);
        }

    //  Handle user input to control spherical light size
        if(wnd.IsKeyPressed(Window::KEY_R)) {
            lightProperties.Radius+=increment;
            printf("R %f\n", lightProperties.Radius);
        } else if(wnd.IsKeyPressed(Window::KEY_F)) {
            lightProperties.Radius-=increment;
            if(lightProperties.Radius < 0) lightProperties.Radius = 0;
            printf("R %f\n", lightProperties.Radius);
        }

    //  Update spherical light data to be used by objects and light pipelines
        spherelight.SetCustomUniformShaderInputData(1,&lightProperties.Intensity, sizeof(lightProperties.Intensity), offsetof(LightProperties, Intensity));
        spherelight.SetCustomUniformShaderInputData(1,&lightProperties.Radius, sizeof(lightProperties.Radius), offsetof(LightProperties, Radius));
        
         auto position = spherelight.GetPosition();
        LightInfo lightInfo = {Vec<4>({position(0),position(1),position(2),1}), lightProperties};
        renderer.SetCustomGlobalUniformShaderData(objPipeline, 1, &lightInfo, sizeof(lightInfo), 0);

    //  Draw Objects
        if (renderer.DrawObject(cube, mainCamera, objPipeline) == false) {
            std::cout << "Draw failed: cube" << std::endl;
            break;
        }
        if (renderer.DrawObject(sphere, mainCamera, objPipeline) == false) {
            std::cout << "Draw failed: sphere" << std::endl;
            break;
        }

        if (renderer.DrawObject(spherelight, mainCamera, lightPipeline) == false) {
            std::cout << "Draw failed: light" << std::endl;
            break;
        }
    //  Present Frame
        if (renderer.PresentFrame() == false) {
            printf("present frame failed\n");
            break;
        }

    //  Window update
        wnd.Update();

    //  Check Window exit event
        if (wnd.CheckExit()) {
            std::cout << "exit" << std::endl;
            break;
        }

    //  Limit maximum framerate to 100fps
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

    //  Report average frame time after "n" frames
        i %= n;
        if (i == 0) {
            profiler.Check("Frame Time", n);
            profiler.Start();
        }
    }

    if(renderer.Cleanup() == false) {
        printf("renderer cleanup failed\n");
        return -1;
    }

    if(wnd.Cleanup()==false) {
        printf("Window cleanup failed\n");
        return -1;
    }

    return 0;
}

#if defined(_WIN32)
    #include <windows.h>
#elif defined(__linux__)
    #include <unistd.h>
#endif
std::string get_bin_dir() {
    static std::string binDir;

    if(binDir.length() > 0) {
        return binDir;
    }

    char binPath[1024];
    #if defined(_WIN32)
        GetModuleFileNameA(NULL, binPath, sizeof(binPath));
    #elif defined(__linux__)
        ssize_t count = readlink("/proc/self/exe", binPath, sizeof(binPath) - 1);
        if (count != -1) {
            binPath[count] = '\0'; // Null-terminate the string
        } else {
            throw std::runtime_error("Failed to retrieve executable path");
        }
    #endif

    binDir = std::filesystem::path(binPath).parent_path().string();
    return binDir;
}