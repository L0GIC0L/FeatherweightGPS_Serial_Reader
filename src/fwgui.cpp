// Demo:   demo.cpp
// Author: Evan Pezent (evanpezent.com)
// Date:   3/26/2021
#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_glfw.h"
#include "../imgui/imgui_impl_opengl3.h"
#include "../implot/implot.h"
#include "../3rdparty/exprtk.hpp"
#include "../imgui/imgui_stdlib.h"
#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include "functions.h"
#include "plotfuncs.h"
#include <thread>
#include <chrono>
#include <atomic>

//********************************************************************************************

//Set the port directory (typically COM3 on windows)
//string port = "/dev/serial/by-id/usb-FTDI_FT230X_Basic_UART_D201170B-if00-port0";

// Map to store the parsed data
std::multimap < std::string, std::string > parsed_data;

//**********************************************************************

bool con_status = 0;
bool open_file = 1;
std::string line = "";
char port[256] = "COM3";
char buffer2[256] = "data.csv";


//*************************************************************************
//Define the velocity atomic types
std::atomic<double> read_vel_x, read_vel_y, read_vel_z;

//Define the time atomic types
std::atomic<double> read_time_ms, read_time_s, read_time_m, read_time_h;

//Define the location atomic types
std::atomic<double> read_altitude, read_longitude, read_latitude, read_sat;
//*************************************************************************

std::atomic<bool> filesave = 0;

//This is a function to read and parse the velocity
void read_and_parse(char port[256], char loc[256])
{

  while ( true )
    {

      if (filesave.load() == true) {
          std::ofstream csv_file ( buffer2 );
          openSerialPort(port);
          while (filesave.load() == true) {

              line = readSerialPort ( "GPS_STAT" );
              parseData ( parsed_data, line );
              saveFile(parsed_data,csv_file);

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
          }
          csv_file.close();
          closeSerialPort();
      } else if (filesave.load() == false) {
          openSerialPort(port);
          while (filesave.load() == false) {
              line = readSerialPort("GPS_STAT");
              parseData(parsed_data, line);

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
          }
          closeSerialPort();
      }

    }
}

int main ( int argc, char const *argv[] )
{
      // Initialize GLFW and create a window
      glfwInit();
      GLFWwindow* window = glfwCreateWindow ( 1280, 720, "Featherweight-GPS", NULL, NULL );
      glfwMakeContextCurrent ( window );
      glfwSwapInterval(1); // Enable vsync

      // Initialize GLEW
      glewExperimental = GL_TRUE;
      glewInit();

      // Setup ImGui context
      IMGUI_CHECKVERSION();
      ImGui::CreateContext();
      ImPlot::CreateContext();
      ImGuiIO& io = ImGui::GetIO();
      ( void ) io;
      io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
      // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
      io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
      //io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
      ImGui::StyleColorsLight();



    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 1.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

      // Setup ImGui GLFW backend
      ImGui_ImplGlfw_InitForOpenGL ( window, true );
      ImGui_ImplOpenGL3_Init ( "#version 330" );

    std::thread t ( &read_and_parse, port, buffer2 );

      // Main loop
      while ( !glfwWindowShouldClose ( window ) )
        {
          glfwPollEvents();

          // Start ImGui frame
          ImGui_ImplOpenGL3_NewFrame();
          ImGui_ImplGlfw_NewFrame();
          ImGui::NewFrame();

          // Create a basic ImGui window
          ImGui::Begin ( "Velocity" );
          livePlot ( read_vel_x.load(), read_vel_y.load(), read_vel_z.load(), "Velocity X","Velocity Y","Velocity Z" );
          ImGui::End();

          ImGui::Begin ( "Altitude" );
          livePlot ( read_altitude.load(), NULL, NULL,"Altitude","","");
            ImGui::Text("Altitude: %.0f", read_altitude.load());
          ImGui::End();

          ImGui::Begin ( "Map",0 );
          TileManager mngr;
          Demo_Map ( mngr );
          ImGui::End();

          ImGui::Begin ( "Settings",0 );

          ImGui::Text("Timestamp: %.0f:%.0f:%.0f.%.0f", read_time_h.load(),read_time_m.load(),read_time_s.load(),read_time_ms.load());

            if (ImGui::Button("Reconnect")) {
                // Do something when the button is clicked
                closeSerialPort();
                if (openSerialPort(port)) {
                    con_status = 1;
                } else {
                    con_status = 0;
                }
            }
            ImGui::Text("Connected = %d", con_status);

            ImGui::InputText("Enter port.", port, sizeof(port), ImGuiInputTextFlags_EnterReturnsTrue);

            if (ImGui::Button("Log File Location")) {
                // Do something when the button is clicked
                if (filesave.load() == 0) {
                    filesave.store(1);
                } else {
                    filesave.store(0);
                }
            }
                ImGui::Text("File = %d", filesave.load());

                ImGui::InputText("Enter file location.", buffer2, sizeof(buffer2),
                                 ImGuiInputTextFlags_EnterReturnsTrue);
          ImGui::End();


          // Render ImGui frame
          ImGui::Render();
          glClearColor ( 0.0f, 0.0f, 0.0f, 1.0f );
          glClear ( GL_COLOR_BUFFER_BIT );
          ImGui_ImplOpenGL3_RenderDrawData ( ImGui::GetDrawData() );

          if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
          {
              GLFWwindow* backup_current_context = glfwGetCurrentContext();
              ImGui::UpdatePlatformWindows();
              ImGui::RenderPlatformWindowsDefault();
              glfwMakeContextCurrent(backup_current_context);
          }

          glfwSwapBuffers ( window );
        }

      t.join();

      // Cleanup
      ImGui_ImplOpenGL3_Shutdown();
      ImGui_ImplGlfw_Shutdown();
      ImPlot::CreateContext();
      ImGui::DestroyContext();
      glfwDestroyWindow ( window );
      glfwTerminate();
      return 0;

}

