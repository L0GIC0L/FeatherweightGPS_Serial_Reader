#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_glfw.h"
#include "../imgui/imgui_impl_opengl3.h"
#include "../implot/implot.h"
#include "../3rdparty/exprtk.hpp"
#include "../imgui/imgui_stdlib.h"
#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "functions.h"
#include "plotfuncs.h"
#include <thread>
#include <atomic>
#include "../common/Fonts/Fonts.h"

//********************************************************************************************

// Map to store the parsed data
std::multimap<std::string, std::string> parsed_data;

//**********************************************************************

//Function variable definitions
std::string line_selection;
char port_selection[256] = "COM987";
char log_file_directory[256] = "data.csv";
bool clear_map = false;


//*************************************************************************
//Define the velocity atomic types
std::atomic<double> read_vel_x, read_vel_d, read_vel_z;

//Define the time atomic types
std::atomic<double> read_time_ms, read_time_s, read_time_m, read_time_h;

//Define the location atomic types
std::atomic<double> read_altitude, read_longitude, read_latitude, read_sat;

std::atomic<bool> connection_status, file_save_status, paused_status, shutdown_thread(false);

//*************************************************************************

//This is a function to read and parse the velocity
int read_and_parse(char real_log_file_dir[256]) {

    static std::ofstream csv_file;

    while (!shutdown_thread.load()) {
        if (connection_status.load() && !shutdown_thread.load()) {

            while (!paused_status.load() && !shutdown_thread.load()) {
                line_selection = readSerialPort("GPS_STAT");
                parseData(parsed_data, line_selection);

                //Retrieve time from map
                read_time_ms.store(retrieveLatest(parsed_data, R"(Milliseconds)"));
                read_time_s.store(retrieveLatest(parsed_data, R"(Seconds)"));
                read_time_m.store(retrieveLatest(parsed_data, R"(Minute)"));
                read_time_h.store(retrieveLatest(parsed_data, R"(Hour)"));

                //Retrieve velocity from map
                read_vel_x.store(retrieveLatest(parsed_data, R"(Horizontal_Velocity)"));
                read_vel_d.store(retrieveLatest(parsed_data, R"(Horizontal_Heading)"));
                read_vel_z.store(retrieveLatest(parsed_data, R"(Vertical_Velocity)"));

                //Retrieve latitude, longitude, and altitude from map
                read_latitude.store(retrieveLatest(parsed_data, R"(Latitude)"));
                read_longitude.store(retrieveLatest(parsed_data, R"(Longitude)"));
                read_sat.store(retrieveLatest(parsed_data, R"(Satellite)"));
                read_altitude.store(retrieveLatest(parsed_data, R"(Altitude)"));

                if (file_save_status.load()) {
                    if (!csv_file.is_open()) {
                        csv_file.open(real_log_file_dir);
                    }
                    saveFile(parsed_data, csv_file);
                } else if (csv_file.is_open()) {
                    csv_file.close();
                }

                if (shutdown_thread.load()) {
                    cout << "EXITING THREAD" << endl;
                    return 0;
                }
            }
        }
    }
    closeSerialPort();
    csv_file.close();
    return 1;
}

int main(int argc, char const *argv[]) {

    // Initialize GLFW and create a window
    glfwInit();
    GLFWwindow *window = glfwCreateWindow(1280, 720, "Featherweight-GPS", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Initialize GLEW
    glewExperimental = GL_TRUE;
    glewInit();
//****************************************************************

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();

    // Setup ImGui context
    ImGuiIO& io = ImGui::GetIO();
    (void) io;


    //Set ImGui flags
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
    //io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport_selection / Platform Windows

    //Setup the styles
    ImGui::GetStyle().AntiAliasedLines = true;
    ImGui::GetStyle().AntiAliasedFill = true;
    ImGui::StyleColorsLight();
    ImGuiStyle &style = ImGui::GetStyle();
    style.WindowMinSize        = ImVec2( 160, 20 );
    style.FramePadding         = ImVec2( 4, 2 );
    style.ItemSpacing          = ImVec2( 6, 2 );
    style.ItemInnerSpacing     = ImVec2( 6, 4 );
    style.WindowRounding       = 4.0f;
    style.FrameRounding        = 2.0f;
    style.IndentSpacing        = 6.0f;
    style.ItemInnerSpacing     = ImVec2( 2, 4 );
    style.ColumnsMinSpacing    = 50.0f;
    style.GrabMinSize          = 14.0f;
    style.GrabRounding         = 4.0f;
    style.ScrollbarSize        = 12.0f;
    style.ScrollbarRounding    = 16.0f;
    ImGui::GetStyle().Colors[ImGuiCol_WindowBg].w = 1.0f; // set window background color alpha to opaque
    ImGui::GetStyle().Colors[ImGuiCol_DockingEmptyBg].w = 0.0f; // set docking empty background color alpha to transparent

    ImFont* font = io.Fonts->AddFontFromMemoryTTF(RobotoMono_Regular_ttf, RobotoMono_Regular_ttf_len ,18.0f);

    // Setup ImGui GLFW backend
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    connection_status.store(false);
    file_save_status.store(false);
    paused_status.store(false);

    // Define the threads
    std::thread read_serial_thread(&read_and_parse, log_file_directory);

    // Main loop

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Start ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowPos(ImVec2(0, 0)); // Set the window position to (0, 0)
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize); // Set the window size to the display size

        ImGui::Begin("Main Window", nullptr,
                     ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse |
                     ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBringToFrontOnFocus);

        ImGui::PushFont(font);

        // Create the dock space
        ImGui::DockSpace(ImGui::GetID("Dockspace Window"), ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);

        // Window contents here
        ImGui::Begin("Velocity");

        livePlot(1,read_vel_x.load(), read_vel_d.load(), read_vel_z.load(),"Horizontal Velocity","Horizontal Heading", "Upward Velocity");

        ImGui::Text("Velocity Data:");
        ImGui::BulletText("Velocity (Horizontal, Vertical): (%.0f, %.0f)", read_vel_x.load(),read_vel_z.load());

        ImGui::Text("Heading:");
        ImGui::BulletText("Heading %.0f", read_vel_d.load());
        ImGui::End();

        ImGui::Begin("Altitude");
        livePlot(2, read_altitude.load(), 0, 0,"Altitude", "", "");
        ImGui::Text("Current Altitude:");
        ImGui::BulletText("Altitude: %.0f", read_altitude.load());
        ImGui::End();

        ImGui::Begin("Map", nullptr);
        TileManager mngr;
        //Demo_Map ( mngr, ((read_latitude.load()+90)/360), ((read_longitude.load()+180)/360) );
        Demo_Map(mngr, lat2tiley(read_latitude.load()), long2tilex(read_longitude.load()), clear_map);
        ImGui::End();

        ImGui::Begin("Settings", nullptr);

        ImGui::Text("Timestamp: %.0f:%.0f:%.0f.%.0f", read_time_h.load(), read_time_m.load(), read_time_s.load(),
                    read_time_ms.load());

        if (ImGui::Button("Reconnect")) {
            // Do something when the button is clicked
            cout << "BUTTON 'RECONNECT' PRESSED" << endl;
            closeSerialPort();
            if (openSerialPort(port_selection)) {
                connection_status.store(true);
            } else {
                connection_status.store(false);
            }
        }
        ImGui::BulletText("Connected = %d", connection_status.load());

        ImGui::Text("Enter the Port Selection Below: ");
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(64,64,128,255));
        ImGui::PushItemWidth(250);
        ImGui::InputText("##Text1", port_selection, sizeof(port_selection),
                         ImGuiInputTextFlags_EnterReturnsTrue);
        ImGui::PopStyleColor();

        ImGui::Text("\n\nEnter Log File Location Below: ");
        if (ImGui::Button("Log File Location")) {
            cout << "BUTTON 'LOG FILE' PRESSED" << endl;
            file_save_status.store(!file_save_status.load());
        }
        ImGui::BulletText("File = %d", file_save_status.load());

        // Particular widget styling
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255,128,128,255));
        ImGui::PushItemWidth(250);
        ImGui::InputText("##text2", log_file_directory, sizeof(log_file_directory),
                         ImGuiInputTextFlags_EnterReturnsTrue);
        ImGui::PopStyleColor();

        ImGui::Text("\n\nPause/Resume: ");
        if (ImGui::Button("Paused Status")) {
            cout << "BUTTON 'PAUSE' PRESSED" << endl;
            paused_status.store(!paused_status.load());
        }
        ImGui::BulletText("Paused = %d", paused_status.load());

        ImGui::Text("\n\nCurrent Coordinates: ( %.5f : %.5f )", read_longitude.load(), read_latitude.load());

        if (ImGui::Button("Clear Map")) {
            cout << "BUTTON 'Clear Map' PRESSED" << endl;
            clear_map = true;
        }

        ImGui::End();

        ImGui::PopFont();

        ImGui::End();

        // Render ImGui frame
        ImGui::Render();
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            GLFWwindow *backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }

        glfwSwapBuffers(window);
    }

    // Cleanup
    shutdown_thread.store(true);
    std::cout << "CLOSING SERIAL PORT" << std::endl;
    ImGui::GetIO().Fonts->ClearFonts();
    std::cout << "WIPING FONTS" << std::endl;
    ImGui_ImplOpenGL3_Shutdown();
    std::cout << "SHUTTING DOWN GL" << std::endl;
    ImGui_ImplGlfw_Shutdown();
    std::cout << "SHUTTING DOWN GLFW" << std::endl;
    ImPlot::DestroyContext();
    std::cout << "KILLING CONTEXT" << std::endl;
    // code that causes the invalid pointer error
    //ImGui::DestroyContext();
    std::cout << "DESTROYING WINDOW" << std::endl;
    glfwDestroyWindow(window);
    std::cout << "TERMINATING GLFW" << std::endl;
    glfwTerminate();
    std::cout << "MERGING THREADS" << std::endl;
    read_serial_thread.join();
    std::cout << "Goodbye, it was a pleasure working with [LIBERTY ROCKETRY]." << std::endl;
    return 0;
}

