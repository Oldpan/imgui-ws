#include "imgui/imgui.h"
#include "imgui-ws/imgui-ws.h"

#include "common.h"

#include <opencv2/opencv.hpp>
#include <sys/stat.h>
#include <dirent.h>
#include <map>
#include <fstream>
#include <iostream>
// #include <iomanip>

struct State {
    State() {
        for (int i = 0; i < 512; ++i) {
            lastKeysDown[i] = false;
        }
    }

    bool showImageWindow = true;

    // client control management
    struct ClientData {
        bool hasControl = false;

        std::string ip = "---";
    };

    // client control
    float tControl_s = 10.0f;
    float tControlNext_s = 0.0f;

    int controlIteration = 0;
    int curIdControl = -1;
    std::map<int, ClientData> clients;

    // client input
    ImVec2 lastMousePos = { 0.0, 0.0 };
    bool  lastMouseDown[5] = { false, false, false, false, false };
    float lastMouseWheel = 0.0;
    float lastMouseWheelH = 0.0;

    std::string lastAddText = "";
    bool lastKeysDown[512];

    void handle(ImGuiWS::Event && event);
    void update();
};

struct ImageShowUtil{

    std::vector<std::string> imagePaths;
    std::string imagePath;
    int imageNum;
    bool imageUpdate = false;
    std::string name;
    void readPath(char* fn);
};

uint32_t g_textureId = 100;

int main(int argc, char ** argv) {
    printf("Usage: %s [port] [http-root]\n", argv[0]);

    int port = 5000;
    std::string httpRoot = "../examples";

    if (argc > 1) port = atoi(argv[1]);
    if (argc > 2) httpRoot = argv[2];

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    io.KeyMap[ImGuiKey_Tab]         = 9;
    io.KeyMap[ImGuiKey_LeftArrow]   = 37;
    io.KeyMap[ImGuiKey_RightArrow]  = 39;
    io.KeyMap[ImGuiKey_UpArrow]     = 38;
    io.KeyMap[ImGuiKey_DownArrow]   = 40;
    io.KeyMap[ImGuiKey_PageUp]      = 33;
    io.KeyMap[ImGuiKey_PageDown]    = 34;
    io.KeyMap[ImGuiKey_Home]        = 36;
    io.KeyMap[ImGuiKey_End]         = 35;
    io.KeyMap[ImGuiKey_Insert]      = 45;
    io.KeyMap[ImGuiKey_Delete]      = 46;
    io.KeyMap[ImGuiKey_Backspace]   = 8;
    io.KeyMap[ImGuiKey_Space]       = 32;
    io.KeyMap[ImGuiKey_Enter]       = 13;
    io.KeyMap[ImGuiKey_Escape]      = 27;
    io.KeyMap[ImGuiKey_A]           = 65;
    io.KeyMap[ImGuiKey_C]           = 67;
    io.KeyMap[ImGuiKey_V]           = 86;
    io.KeyMap[ImGuiKey_X]           = 88;
    io.KeyMap[ImGuiKey_Y]           = 89;
    io.KeyMap[ImGuiKey_Z]           = 90;

    io.MouseDrawCursor = false;

    ImGui::StyleColorsDark();
    ImGui::GetStyle().AntiAliasedFill = false;
    ImGui::GetStyle().AntiAliasedLines = false;
    ImGui::GetStyle().WindowRounding = 0.0f;
    ImGui::GetStyle().ScrollbarRounding = 0.0f;

    // setup imgui-ws
    ImGuiWS imguiWS;
    imguiWS.init(port, (httpRoot + "/demo-null").c_str());

    // prepare font texture
    {
        unsigned char * pixels;
        int width, height;
        ImGui::GetIO().Fonts->GetTexDataAsAlpha8(&pixels, &width, &height);
        imguiWS.setTexture(0, ImGuiWS::Texture::Type::Alpha8, width, height, (const char *) pixels);
    }

    cv::Mat temp_img = cv::imread("/home/homework/data/TXFLv2/train/jielong/jielong_1_new_866376e436ddc06f04129f79600c5196.jpg");
    cv::cvtColor(temp_img, temp_img, cv::COLOR_BGR2RGB);
    cv::resize(temp_img, temp_img, cv::Size(1024,1024));
    // temp_img.convertTo(temp_img, CV_8SC3);

    int width = temp_img.cols;
    int height = temp_img.rows;
    imguiWS.setTexture(g_textureId, ImGuiWS::Texture::Type::RGB24, width, height, (const char *) temp_img.data);

    VSync vsync;
    State state;

    while (true) {
        // websocket event handling
        auto events = imguiWS.takeEvents();
        for (auto & event : events) {
            state.handle(std::move(event));
        }
        state.update();

        io.DisplaySize = ImVec2(1600, 1200);
        io.DeltaTime = vsync.delta_s();

        ImGui::NewFrame();

        // Control/Debug window
        {
            ImGui::Begin("FPS");
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::Checkbox("Image Window", &state.showImageWindow);
            ImGui::End();
        }

        if(state.showImageWindow){
            ImageShowUtil imgsu;
            static char image_path[128] = "/";
            
            ImGui::Begin("Image Show Window");
            ImGui::InputTextWithHint("image path", "input absolute image path", image_path, IM_ARRAYSIZE(image_path));
            imgsu.imageUpdate |= ImGui::Button("update");
            if(imgsu.imageUpdate){
                if(imgsu.imagePath != std::string(image_path)){
                    try{
                        imgsu.readPath(image_path);
                    }
                    catch(const char* msg){
                        std::cout << "error: " << msg << "\n";
                    }
                    // 
                }

                // for(auto& img_path:imgsu.imagePaths){
                    
                // }

            }

        
            ImGui::End();
        
        }

        ImGui::Begin("Some random images");
        ImGui::Image((void *)(intptr_t)g_textureId, { width, height }, ImVec2(0,0), ImVec2(1,1), ImVec4(1.0f,1.0f,1.0f,1.0f), ImVec4(1.0f,1.0f,1.0f,0.5f));
        ImGui::End();

        // show connected clients
        ImGui::SetNextWindowPos({ 10, 10 } , ImGuiCond_Always);
        ImGui::SetNextWindowSize({ 400, 300 } , ImGuiCond_Always);
        ImGui::Begin((std::string("WebSocket clients (") + std::to_string(state.clients.size()) + ")").c_str(), nullptr, ImGuiWindowFlags_NoCollapse);
        ImGui::Text(" Id   Ip addr");
        for (auto & [ cid, client ] : state.clients) {
            ImGui::Text("%3d : %s", cid, client.ip.c_str());
            if (client.hasControl) {
                ImGui::SameLine();
                ImGui::TextDisabled(" [has control for %4.2f seconds]", state.tControlNext_s - ImGui::GetTime());
            }
        }
        ImGui::End();

        // generate ImDrawData
        ImGui::Render();

        // store ImDrawData for asynchronous dispatching to WS clients
        imguiWS.setDrawData(ImGui::GetDrawData());

        // if not clients are connected, just sleep to save CPU
        do {
            vsync.wait();
        } while (imguiWS.nConnected() == 0);
    }

    ImGui::DestroyContext();

    return 0;
}

void State::handle(ImGuiWS::Event && event) {
    switch (event.type) {
        case ImGuiWS::Event::Connected:
            {
                clients[event.clientId].ip = event.ip;
            }
            break;
        case ImGuiWS::Event::Disconnected:
            {
                clients.erase(event.clientId);
            }
            break;
        case ImGuiWS::Event::MouseMove:
            {
                if (event.clientId == curIdControl) {
                    lastMousePos.x = event.mouse_x;
                    lastMousePos.y = event.mouse_y;
                }
            }
            break;
        case ImGuiWS::Event::MouseDown:
            {
                if (event.clientId == curIdControl) {
                    lastMouseDown[event.mouse_but] = true;
                    lastMousePos.x = event.mouse_x;
                    lastMousePos.y = event.mouse_y;
                }
            }
            break;
        case ImGuiWS::Event::MouseUp:
            {
                if (event.clientId == curIdControl) {
                    lastMouseDown[event.mouse_but] = false;
                    lastMousePos.x = event.mouse_x;
                    lastMousePos.y = event.mouse_y;
                }
            }
            break;
        case ImGuiWS::Event::MouseWheel:
            {
                if (event.clientId == curIdControl) {
                    lastMouseWheelH = event.wheel_x;
                    lastMouseWheel  = event.wheel_y;
                }
            }
            break;
        case ImGuiWS::Event::KeyUp:
            {
                if (event.clientId == curIdControl) {
                    if (event.key > 0) {
                        lastKeysDown[event.key] = false;
                    }
                }
            }
            break;
        case ImGuiWS::Event::KeyDown:
            {
                if (event.clientId == curIdControl) {
                    if (event.key > 0) {
                        lastKeysDown[event.key] = true;
                    }
                }
            }
            break;
        case ImGuiWS::Event::KeyPress:
            {
                if (event.clientId == curIdControl) {
                    lastAddText.resize(1);
                    lastAddText[0] = event.key;
                }
            }
            break;
        default:
            {
                printf("Unknown input event\n");
            }
    }
}

void State::update() {
    if (clients.size() > 0 && (clients.find(curIdControl) == clients.end() || ImGui::GetTime() > tControlNext_s)) {
        if (clients.find(curIdControl) != clients.end()) {
            clients[curIdControl].hasControl = false;
        }
        int k = ++controlIteration % clients.size();
        auto client = clients.begin();
        std::advance(client, k);
        client->second.hasControl = true;
        curIdControl = client->first;
        tControlNext_s = ImGui::GetTime() + tControl_s;
    }

    if (clients.size() == 0) {
        curIdControl = -1;
    }

    if (curIdControl > 0) {
        ImGui::GetIO().MousePos = lastMousePos;
        ImGui::GetIO().MouseWheelH = lastMouseWheelH;
        ImGui::GetIO().MouseWheel = lastMouseWheel;
        ImGui::GetIO().MouseDown[0] = lastMouseDown[0];
        ImGui::GetIO().MouseDown[1] = lastMouseDown[1];
        ImGui::GetIO().MouseDown[2] = lastMouseDown[2];
        ImGui::GetIO().MouseDown[3] = lastMouseDown[3];
        ImGui::GetIO().MouseDown[4] = lastMouseDown[4];

        if (lastAddText.size() > 0) {
            ImGui::GetIO().AddInputCharactersUTF8(lastAddText.c_str());
        }

        for (int i = 0; i < 512; ++i) {
            ImGui::GetIO().KeysDown[i] = lastKeysDown[i];
        }

        lastMouseWheelH = 0.0;
        lastMouseWheel = 0.0;
        lastAddText = "";
    }
}


void ImageShowUtil::readPath(char* fn){

    struct stat name_stat;
    if (stat(fn, &name_stat) != 0) {
        std::cerr << "Failed to find '" << std::string(fn)
                << "': " << strerror(errno) << std::endl;
        exit(1);
    }
    // 读取图像名称并且放入数组 image_filenames
    if (name_stat.st_mode & S_IFDIR) {
        const std::string dirname = fn;
        DIR* dir_ptr = opendir(dirname.c_str());
        struct dirent* d_ptr;
        while ((d_ptr = readdir(dir_ptr)) != NULL) {
            const std::string filename = d_ptr->d_name;
            if ((filename != ".") && (filename != "..")) {
                imagePaths.push_back(dirname + "/" + filename);
            }
        }
        closedir(dir_ptr);
    } else {
        imagePaths.push_back(fn);
    }

}