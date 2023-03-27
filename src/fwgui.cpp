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

//********************************************************************************************

// Map to store the parsed data
std::multimap<std::string, std::string> parsed_data;

//**********************************************************************

//Function variable definitions
std::string line_selection;
char port_selection[256] = "/dev/serial/by-id/usb-Silicon_Labs_CP2102_USB_to_UART_Bridge_Controller_0001-if00-port0";
char log_file_directory[256] = "data.csv";
double delta_time_new = 0;
double delta_time_olde = 0;


//*************************************************************************
//Define the velocity atomic types
std::atomic<double> read_vel_x, read_vel_y, read_vel_z;

//Define the time atomic types
std::atomic<double> read_time_ms, read_time_s, read_time_m, read_time_h, read_delta_time;

//Define the location atomic types
std::atomic<double> read_altitude, read_longitude, read_latitude, read_sat;

std::atomic<bool> connection_status, filesave_status, paused_status, shutdown_thread(false);

//*************************************************************************

//This is a function to read and parse the velocity
int read_and_parse(char rport_selection[256], char rlog_file_directory[256]) {

    static std::ofstream csv_file;

    while (!shutdown_thread.load()) {
        if (connection_status.load()) {
            openSerialPort(rport_selection);

            while (!paused_status.load() && !shutdown_thread.load()) {
                line_selection = readSerialPort("GPS_STAT");
                parseData(parsed_data, line_selection);

                //Retrieve time from map
                read_time_ms.store(retrieveLatest(parsed_data, R"(Milliseconds)"));
                read_time_s.store(retrieveLatest(parsed_data, R"(Seconds)"));
                read_time_m.store(retrieveLatest(parsed_data, R"(Minute)"));
                read_time_h.store(retrieveLatest(parsed_data, R"(Hour)"));

                //Retrieve velocity from map
                read_vel_x.store(retrieveLatest(parsed_data, R"(VelocityX)"));
                read_vel_y.store(retrieveLatest(parsed_data, R"(VelocityY)"));
                read_vel_z.store(retrieveLatest(parsed_data, R"(VelocityZ)"));

                //Retrieve latitude, longitude, and altitude from map
                read_latitude.store(retrieveLatest(parsed_data, R"(Latitude)"));
                read_longitude.store(retrieveLatest(parsed_data, R"(Longitude)"));
                read_sat.store(retrieveLatest(parsed_data, R"(Satellite)"));
                read_altitude.store(retrieveLatest(parsed_data, R"(Altitude)"));

                if (filesave_status.load()) {
                    if (!csv_file.is_open()) {
                        csv_file.open(rlog_file_directory);
                    }
                    saveFile(parsed_data, csv_file);
                } else if (csv_file.is_open()) {
                    csv_file.close();
                }
            }
        }

        if (shutdown_thread.load()) {
            if (csv_file.is_open()) {
                csv_file.close();
            }
            closeSerialPort();
            return 0;
        }
    }
    return 0;
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

    // Setup ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void) io;

    //Set ImGui flags
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
    //io.ConfigFlags |= ImGuiConfigFlags_Viewport_selectionsEnable;         // Enable Multi-Viewport_selection / Platform Windows

    //Setup the styles
    ImGui::GetStyle().AntiAliasedLines = true;
    ImGui::GetStyle().AntiAliasedFill = true;
    ImGui::StyleColorsLight();
    ImGuiStyle &style = ImGui::GetStyle();
    style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    style.WindowRounding = 5.0f; // Set the window rounding radius to 5.

    // Setup ImGui GLFW backend
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    connection_status.store(false);
    filesave_status.store(false);
    paused_status.store(false);

    // Define the threads
    std::thread read_serial_thread(&read_and_parse, port_selection, log_file_directory);

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
                     ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBringToFrontOnFocus |
                     ImGuiWindowFlags_NoBackground);

        if (ImGui::BeginMenuBar()) {
            // Menu bar contents here
            ImGui::EndMenuBar();
        }

        // Create the dock space
        ImGui::DockSpace(ImGui::GetID("Dockspace Window"), ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);

        // Window contents here
        ImGui::Begin("Velocity");
        delta_time_olde = delta_time_new;
        delta_time_new = read_time_s.load();
        if (delta_time_new-delta_time_olde > 0) {
            cout << "Variable changed from " << delta_time_olde << " to " << delta_time_new << endl;
        }

        livePlot(1,read_vel_x.load(), read_vel_y.load(), read_vel_z.load(),"Velocity X","Velocity Y", "Velocity Z");

        ImGui::BulletText("This is the Velocity Data");
        ImGui::BulletText("Velocity X: %.0f", read_vel_x.load());
        ImGui::End();

        ImGui::Begin("Altitude");
        livePlot(2, read_altitude.load(), NULL, NULL,"Altitude", "", "");
        ImGui::BulletText("Altitude: %.0f", read_altitude.load());
        ImGui::BulletText("This is the Altitude Data");
        ImGui::End();

        ImGui::Begin("Map", nullptr);
        TileManager mngr;
        //Demo_Map ( mngr, ((read_latitude.load()+90)/360), ((read_longitude.load()+180)/360) );
        Demo_Map(mngr, ((37.35242 + 90) / 360), ((-79.18018 + 180) / 360));
        ImGui::End();

        ImGui::Begin("Settings", nullptr);

        ImGui::Text("Timestamp: %.0f:%.0f:%.0f.%.0f", read_time_h.load(), read_time_m.load(), read_time_s.load(),
                    read_time_ms.load());

        if (ImGui::Button("Reconnect")) {
            // Do something when the button is clicked
            cout << "BUTTON 'RECONNECT' PRESSED" << endl;
            closeSerialPort();
            if (openSerialPort(port_selection) == true) {
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
            filesave_status.store(!filesave_status.load());
        }
        ImGui::BulletText("File = %d", filesave_status.load());

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
        ImGui::End();

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
    closeSerialPort();
    shutdown_thread.store(true);
    std::cout << "CLOSING SERIAL PORT" << std::endl;
    ImGui_ImplOpenGL3_Shutdown();
    std::cout << "SHUTTING DOWN GL" << std::endl;
    ImGui_ImplGlfw_Shutdown();
    std::cout << "SHUTTING DOWN GL" << std::endl;
    ImPlot::DestroyContext();
    std::cout << "KILLING CONTEXT" << std::endl;
    ImGui::DestroyContext();
    std::cout << "DESTROYING WINDOW" << std::endl;
    glfwDestroyWindow(window);
    std::cout << "TERMINATING GLFW" << std::endl;
    glfwTerminate();
    std::cout << "MERGING THREADS" << std::endl;
    read_serial_thread.join();
    std::cout << "Goodbye, it was a pleasure working with [LIBERTY ROCKETRY]." << std::endl;
    return 0;
}

