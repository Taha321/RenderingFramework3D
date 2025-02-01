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
    if(wnd.Initialize(false, windowWidth, windowHeight , "Renderer Base Test") == false) {
        printf("failed to initialize window\n");
        return -1;
    }

//  Create Renderer
    Renderer renderer;
    if(renderer.Initialize(wnd)==false) {
        printf("failed to initialize renderer\n");
        return -1; 
    }

//  Create Camera
    Camera mainCamera({ 0,0,windowWidth, windowHeight });

//  Object array
    static constexpr unsigned nObj = 100;
    std::array<WorldObject, nObj> objList;
    std::array<Vec<3>, nObj> axes;
    std::array<Vec<3>, nObj> velocities;

//  Initialize Objects
    unsigned idx = 0;
    Mesh cubeMesh = Mesh::Cube(renderer);
    if(cubeMesh.LoadMesh() == false) {
        printf("failed to load mesh to the GPU\n");
        return -1;
    }
    for (auto& obj : objList) {
        obj = WorldObject(cubeMesh);
        obj.SetPosition(Vec<3>({ RandomFloat(150, -150), RandomFloat(150, -150), RandomFloat(150, -150) }));
        obj.GetMaterial().colour = Vec<4>({ 0.9047,0.0381,0,1 });

        obj.SetScale(RandomFloat(4, 10), RandomFloat(4, 10), RandomFloat(4, 10));

        Vec<3> axis({ RandomFloat(1, -1) ,RandomFloat(1, -1) ,RandomFloat(1, -1) });
        obj.Rotate(axis, RandomFloat(2 * PI, -2 * PI));
        axes[idx] = axis;

        velocities[idx] = Vec<3>({ RandomFloat(0.1, -0.1) ,RandomFloat(0.1, -0.1) ,RandomFloat(0.1, -0.1) });

        idx++;
    }

//  set default global uniform shader inputs including directional and ambient lights
    renderer.SetLightDirection(PIPELINE_SHADED, Vec<3>({1,0,0}));
    renderer.SetLightColour(PIPELINE_SHADED, Vec<4>({1,1,1,1}));
    renderer.SetLightIntensity(PIPELINE_SHADED, 1);
    renderer.SetAmbientLightIntensity(PIPELINE_SHADED, 0.06);

//  set cursor visibility
    wnd.SetMouseVisibility(false);

//  main loop
    unsigned pipeline = PIPELINE_SHADED;
    unsigned n = 1000;
    TimeProfiler profiler;
    profiler.Start();
    for (int i = 0;; i++) {
    //  handle FPV camera rotation and movement
        auto disp = wnd.GetMouseDisplacement();

        if (wnd.CheckKeyPressEvent(Window::KEY_LCTRL)) {
            wnd.SetMouseVisibility(true);
        }
        if (wnd.CheckKeyReleaseEvent(Window::KEY_LCTRL)) {
            wnd.SetMouseVisibility(false);
        }
        
        float scale = 0.01;
        Vec<3> axis = mainCamera.GetCameraAxisX() * disp(1) + mainCamera.GetCameraAxisY() * disp(0);
        float len = sqrt(axis.LenSqr());
        if (wnd.IsKeyPressed(Window::KEY_LCTRL)==false) {
            if (len > 0.001) {
                mainCamera.Rotate(axis, len * scale);
            }
        }
 
        scale = 0.8;
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
    
    //  Tab key to toggle between 3 default pipelines: Shaded, Wireframe, Unshaded
        if(wnd.CheckKeyPressEvent(Window::KEY_TAB)) {
            if(pipeline==PIPELINE_SHADED) { 
                pipeline = PIPELINE_WIREFRAME;
                for(auto& obj : objList) {
                    obj.SetBackFaceCulling(false);
                }
            }
            else if(pipeline==PIPELINE_WIREFRAME) {
                pipeline = PIPELINE_UNSHADED;
                for(auto& obj : objList) {
                    obj.SetBackFaceCulling(true);
                }
            }
            else if(pipeline==PIPELINE_UNSHADED) {
                pipeline = PIPELINE_SHADED;
                for(auto& obj : objList) {
                    obj.SetBackFaceCulling(true);
                }
            }
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

    //  Objects Rotated and Rendered
        unsigned idx = 0;
        for (auto& obj : objList) {
            obj.Rotate(axes[idx], PI / 400);
            if (renderer.DrawObject(obj, mainCamera, pipeline) == false) {
                std::cout << "Draw failed: object-" << idx << std::endl;
                break;
            }
            idx++;
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