#include <iostream>
#include <chrono>
#include <math.h>
#include <string>
#include <array>
#include <thread>
#include "matrix.h"
#include "window.h"
#include "renderer.h"
#include "timeprofiler.h"


using namespace std::chrono;

using namespace RenderingFramework3D;
using namespace MathUtil;



constexpr float planeWidth = 500, planeHeight = 500;
constexpr unsigned planeDivX = 50, planeDivY = 50;
constexpr float planeSegWidth = planeWidth/planeDivX, planeSegHeight = planeHeight/planeDivY;
constexpr float waveLengthX = planeWidth/2;
constexpr float waveLengthZ = planeHeight/3;
constexpr float waveAmplitude = 20;
constexpr float waveFreq = 0.4;

static int renderer_test();

int main() {
    return renderer_test();
}

void random_init() {
    srand(time(0));
}

float RandomFloat(float max, float min) {
    return ((max - min) * rand()) / (float)(RAND_MAX)+min;
}

static int renderer_test() {
    unsigned windowWidth=1000, windowHeight=800;

    random_init();

//  Create Window
    Window wnd;
    if(wnd.Initialize(false, windowWidth, windowHeight , "Dynamic Mesh Test") == false) {
        printf("failed to initialize window\n");
        return -1;
    }

//  Create Renderer
    Renderer renderer;
    if(renderer.Initialize(wnd)==false) {
        printf("failed to initialize renderer\n");
        return -1; 
    }

//  Set Global Uniform Shader Input Data
    renderer.SetLightDirection(Vec<3>({0, -1, -1}));
    renderer.SetLightIntensity(0.1);
    renderer.SetAmbientLightIntensity(0.06);

//  Create Camera
    Camera mainCamera({ 0,0,windowWidth, windowHeight });
    mainCamera.Move(Vec<3>({0, 0, -250}));

    Mesh cubeMesh = Mesh::Cube(renderer);
    cubeMesh.LoadMesh();

//  Create plane object
    WorldObject cube(cubeMesh);
    cube.SetScale(50, 50, 100);
    cube.GetMaterial().colour = Vec<4>({0.2,0.6,0.7,1});
    cube.GetMaterial().specularConstant= 0.3;
    cube.GetMaterial().shininess = 10;
    cube.SetBackFaceCulling(false);
 
//  set cursor visibility
    wnd.SetMouseVisibility(false);

//  main loop
    unsigned n = 1000;
    TimeProfiler profiler;
    TimeProfiler simTimer;
    profiler.Start();
    simTimer.Start(); 
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

    // P key to switch camera projection mode
        if(wnd.CheckKeyPressEvent(Window::KEY_P)) {
            auto curMode = mainCamera.GetProjectionMode();
            switch(curMode) {
                case PROJ_MODE_ISOMETRIC:
                    mainCamera.SetProjectionMode(PROJ_MODE_PERSPECTIVE);
                    break;
                case PROJ_MODE_PERSPECTIVE:
                    mainCamera.SetProjectionMode(PROJ_MODE_ISOMETRIC);
                    break;
            }
        }
        static constexpr float maxX = PI/2;
        static constexpr float maxY = PI/2;
        static constexpr float maxZ = PI/2;
        static constexpr float inc = 0.01;
        
        static Vec<3> angles({0,0,0});

        if(angles(0) < maxX) {
            angles(0)+=inc;
        }
        if(angles(0) >= maxX && angles(1) < maxY) {
            angles(1)+=inc;
        }
        if(angles(1) >= maxY && angles(2) < maxZ) {
            angles(2) += inc;
        }
        if(angles(2) >= maxZ) {
            angles = Vec<3>({0,0,0});
        }
        cube.SetOrientationEulerXYZ(angles);
        
        if(renderer.DrawObject(cube, mainCamera)==false) {
            printf("failed to draw object\n");
            break;
        }
    
    //  Present frame
        if (renderer.PresentFrame() == false) {
            printf("present frame failed\n");
            break;
        }

    //  Window Update
        wnd.Update();

    //  Reset Camera View Port if window resized
        if (wnd.IsResized()) {
            mainCamera.SetViewPort({ 0,0,wnd.GetWidth(), wnd.GetHeight() });
        }

    //  Check Window exit event to exit main loop
        if (wnd.CheckExit()) {
            std::cout << "exit" << std::endl;
            break;
        }

    //  Limit maximum framerate to 100fps
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

    //  Report average frame duration after "n" frames
        i %= n;
        if (i == 0) {
            profiler.Check("Frame Time", n);
            profiler.Start();
        }
    }

//  Renderer Cleanup
    if(renderer.Cleanup() == false) {
        printf("Renderer cleanup failed");
        return -1;
    }

//  Window Cleanup
    if(wnd.Cleanup() == false) {
        printf("Window Cleanup Failed\n");
        return -1;

    }
    return 0;
}