#pragma once

#pragma once

#pragma once

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
#include "../3rdparty/curl/curl.h"
#include "functions.h"
#include <string>
#include <filesystem>
#include <iostream>
#include <map>
#include <cmath>
#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <stdexcept>
#include <tuple>
#include <atomic>

#include "../common/Image.h"

namespace fs = std::filesystem;

#ifdef _MSC_VER
#define sprintf sprintf_s
#endif

#ifndef PI
#define PI 3.14159265358979323846
#endif

#define CHECKBOX_FLAG(flags, flag) ImGui::CheckboxFlags(#flag, (unsigned int*)&flags, flag)

#define TILE_SERVER   "https://a.tile.openstreetmap.org/" // the tile map server url
#define TILE_SIZE     256                                 // the expected size of tiles in pixels, e.g. 256x256px
#define MAX_ZOOM      19                                  // the maximum zoom level provided by the server
#define MAX_THREADS   2                                   // the maximum threads to use for downloading tiles (OSC strictly forbids more than 2)
#define USER_AGENT    "ImMaps/0.1"                        // change this to represent your own app if you extend this code

#define PI 3.14159265359

int long2tilex ( double lon, int z )
{
  return ( int ) ( floor ( ( lon + 180.0 ) / 360.0 * ( 1 << z ) ) );
}

int lat2tiley ( double lat, int z )
{
  double latrad = lat * PI/180.0;
  return ( int ) ( floor ( ( 1.0 - asinh ( tan ( latrad ) ) / PI ) / 2.0 * ( 1 << z ) ) );
}

double tilex2long ( int x, int z )
{
  return x / ( double ) ( 1 << z ) * 360.0 - 180;
}

double tiley2lat ( int y, int z )
{
  double n = PI - 2.0 * PI * y / ( double ) ( 1 << z );
  return 180.0 / PI * atan ( 0.5 * ( exp ( n ) - exp ( -n ) ) );
}

struct TileCoord
{
  int z; // zoom    [0......20]
  int x; // x index [0...z^2-1]
  int y; // y index [0...z^2-1]
  inline std::string subdir() const
  {
    return std::to_string ( z ) + "/" + std::to_string ( x ) + "/";
  }
  inline std::string dir() const
  {
    return "tiles/" + subdir();
  }
  inline std::string file() const
  {
    return std::to_string ( y ) + ".png";
  }
  inline std::string path() const
  {
    return dir() + file();
  }
  inline std::string url() const
  {
    return TILE_SERVER + subdir() + file();
  }
  inline std::string label() const
  {
    return subdir() + std::to_string ( y );
  }
  std::tuple<ImPlotPoint,ImPlotPoint> bounds() const
  {
    double n = std::pow ( 2,z );
    double t = 1.0 / n;
    return
    {
      { x*t, ( 1+y )*t },
      { ( 1+x )*t, ( y )*t   }
    };
  }
};

bool operator< ( const TileCoord& l, const TileCoord& r )
{
  if ( l.z < r.z )  return true;
  if ( l.z > r.z )  return false;
  if ( l.x < r.x )  return true;
  if ( l.x > r.x )  return false;
  if ( l.y < r.y )  return true;
  if ( l.y > r.y )  return false;
  return false;
}

enum TileState : int
{
  Unavailable = 0, // tile not available
  Loaded,          // tile has been loaded into  memory
  Downloading,     // tile is downloading from server
  OnDisk           // tile is saved to disk, but not loaded into memory
};

typedef Image TileImage;

struct Tile
{
  Tile() : state ( TileState::Unavailable ) {  }
  Tile ( TileState s ) : state ( s ) { }
  TileState state;
  TileImage image;
};

size_t curl_write_cb ( void *ptr, size_t size, size_t nmemb, void *userdata )
{
  FILE *stream = ( FILE * ) userdata;
  if ( !stream )
    {
      printf ( "No stream\n" );
      return 0;
    }
  size_t written = fwrite ( ( FILE * ) ptr, size, nmemb, stream );
  return written;
}

class TileManager
{
public:

  TileManager()
  {
    start_workers();
  }

  inline ~TileManager()
  {
    {
      std::unique_lock<std::mutex> lock ( m_queue_mutex );
      m_stop = true;
    }
    m_condition.notify_all();
    for ( std::thread &worker: m_workers )
      worker.join();
  }

  const std::vector<std::pair<TileCoord, std::shared_ptr<Tile>>>& get_region ( ImPlotRect view, ImVec2 pixels )
  {
    double min_x = std::clamp ( view.X.Min, 0.0, 1.0 );
    double min_y = std::clamp ( view.Y.Min, 0.0, 1.0 );
    double size_x = std::clamp ( view.X.Size(),0.0,1.0 );
    double size_y = std::clamp ( view.Y.Size(),0.0,1.0 );

    double pix_occupied_x = ( pixels.x / view.X.Size() ) * size_x;
    double pix_occupied_y = ( pixels.y / view.Y.Size() ) * size_y;
    double units_per_tile_x = view.X.Size() * ( TILE_SIZE / pix_occupied_x );
    double units_per_tile_y = view.Y.Size() * ( TILE_SIZE / pix_occupied_y );

    int z    = 0;
    double r = 1.0 / pow ( 2,z );
    while ( r > units_per_tile_x && r > units_per_tile_y && z < MAX_ZOOM )
      r = 1.0 / pow ( 2,++z );

    m_region.clear();
    if ( !append_region ( z, min_x, min_y, size_x, size_y ) && z > 0 )
      {
        append_region ( --z, min_x, min_y, size_x, size_y );
        std::reverse ( m_region.begin(),m_region.end() );
      }
    return m_region;
  }

  std::shared_ptr<Tile> request_tile ( TileCoord coord )
  {
    std::lock_guard<std::mutex> lock ( m_tiles_mutex );
    if ( m_tiles.count ( coord ) )
      return get_tile ( coord );
    else if ( fs::exists ( coord.path() ) )
      return load_tile ( coord );
    else
      download_tile ( coord );
    return nullptr;
  }

  int tiles_loaded() const
  {
    return m_loads;
  }
  int tiles_downloaded() const
  {
    return m_downloads;
  }
  int tiles_cached() const
  {
    return m_loads - m_downloads;
  }
  int tiles_failed() const
  {
    return m_fails;
  }
  int threads_working() const
  {
    return m_working;
  }

private:

  bool append_region ( int z, double min_x, double min_y, double size_x, double size_y )
  {
    int k = pow ( 2,z );
    double r = 1.0 / k;
    int xa = min_x * k;
    int xb = xa + ceil ( size_x / r ) + 1;
    int ya = min_y * k;
    int yb = ya + ceil ( size_y / r ) + 1;
    xb = std::clamp ( xb,0,k );
    yb = std::clamp ( yb,0,k );
    bool covered = true;
    for ( int x = xa; x < xb; ++x )
      {
        for ( int y = ya; y < yb; ++y )
          {
            TileCoord coord{z,x,y};
            std::shared_ptr<Tile> tile = request_tile ( coord );
            m_region.push_back ( {coord,tile} );
            if ( tile == nullptr || tile->state != TileState::Loaded )
              covered = false;
          }
      }
    return covered;
  }

  void download_tile ( TileCoord coord )
  {
    auto dir = coord.dir();
    fs::create_directories ( dir );
    if ( fs::exists ( dir ) )
      {
        m_tiles[coord] = std::make_shared<Tile> ( Downloading );
        {
          std::unique_lock<std::mutex> lock ( m_queue_mutex );
          m_queue.emplace ( coord );
        }
        m_condition.notify_one();
      }
  }

  std::shared_ptr<Tile> get_tile ( TileCoord coord )
  {
    if ( m_tiles[coord]->state == Loaded )
      return m_tiles[coord];
    else if ( m_tiles[coord]->state == OnDisk )
      return load_tile ( coord );
    return nullptr;
  }

  std::shared_ptr<Tile> load_tile ( TileCoord coord )
  {
    auto path = coord.path();
    if ( !m_tiles.count ( coord ) )
      m_tiles[coord] = std::make_shared<Tile>();
    if ( m_tiles[coord]->image.LoadFromFile ( path.c_str() ) )
      {
        m_tiles[coord]->state = TileState::Loaded;
        m_loads++;
        return m_tiles[coord];
      }
    m_fails++;
    printf ( "TileManager[00]: Failed to load \"%s\"\n", path.c_str() );
    if ( !fs::remove ( path ) )
      printf ( "TileManager[00]: Failed to remove \"%s\"\n", path.c_str() );
    printf ( "TileManager[00]: Removed \"%s\"\n",path.c_str() );
    m_tiles.erase ( coord );
    return nullptr;
  }

  void start_workers()
  {
    for ( int thrd = 1; thrd < MAX_THREADS+1; ++thrd )
      {
        m_workers.emplace_back (
          [this, thrd]
        {
          //printf ( "TileManager[%02d]: Thread started\n",thrd );
          CURL* curl = curl_easy_init();
          curl_easy_setopt ( curl, CURLOPT_WRITEFUNCTION, curl_write_cb );
          curl_easy_setopt ( curl, CURLOPT_FOLLOWLOCATION, 1 );
          curl_easy_setopt ( curl, CURLOPT_USERAGENT, USER_AGENT );
          for ( ;; )
            {
              TileCoord coord;
              {
                std::unique_lock<std::mutex> lock ( m_queue_mutex );
                m_condition.wait ( lock,
                [this] { return m_stop || !m_queue.empty(); } );
                if ( m_stop && m_queue.empty() )
                  {
                    curl_easy_cleanup ( curl );
                    //printf ( "TileManager[%02d]: Thread terminated\n",thrd );
                    return;
                  }
                coord = std::move ( m_queue.front() );
                m_queue.pop();
              }
              m_working++;
              bool success = true;
              auto dir = coord.dir();
              auto path = coord.path();
              auto url = coord.url();
              FILE *fp = fopen ( coord.path().c_str(), "wb" );
              if ( fp )
                {
                  curl_easy_setopt ( curl, CURLOPT_URL, url.c_str() );
                  curl_easy_setopt ( curl, CURLOPT_WRITEDATA, fp );
                  CURLcode cc = curl_easy_perform ( curl );
                  fclose ( fp );
                  if ( cc != CURLE_OK )
                    {
                      printf ( "TileManager[%02d]: Failed to download: \"%s\"\n", thrd, url.c_str() );
                      long rc = 0;
                      curl_easy_getinfo ( curl, CURLINFO_RESPONSE_CODE, &rc );
                      if ( ! ( ( rc == 200 || rc == 201 ) && rc != CURLE_ABORTED_BY_CALLBACK ) )
                        printf ( "TileManager[%02d]: Response code: %d\n",thrd,rc );
                      fs::remove ( coord.path() );
                      success = false;
                    }
                }
              else
                {
                  printf ( "TileManager[%02d]: Failed to open or create file \"%s\"\n",thrd, path.c_str() );
                  success = false;
                }
              if ( success )
                {
                  m_downloads++;
                  std::lock_guard<std::mutex> lock ( m_tiles_mutex );
                  m_tiles[coord]->state = OnDisk;
                }
              else
                {
                  m_fails++;
                  std::lock_guard<std::mutex> lock ( m_tiles_mutex );
                  m_tiles.erase ( coord );
                }
              m_working--;
            }
        }
        );
      }
  }

  std::atomic<int> m_loads     = 0;
  std::atomic<int> m_downloads = 0;
  std::atomic<int> m_fails     = 0;
  std::atomic<int> m_working   = 0;
  std::map<TileCoord,std::shared_ptr<Tile>> m_tiles;
  std::mutex m_tiles_mutex;
  std::vector<std::pair<TileCoord, std::shared_ptr<Tile>>> m_region;
  std::vector<std::thread> m_workers;
  std::queue<TileCoord> m_queue;
  std::mutex m_queue_mutex;
  std::condition_variable m_condition;
  bool m_stop = false;
};

// utility structure for realtime plot
struct ScrollingBuffer
{
  int MaxSize;
  int Offset;
  ImVector<ImVec2> Data;
  ScrollingBuffer ( int max_size = 2000 )
  {
    MaxSize = max_size;
    Offset  = 0;
    Data.reserve ( MaxSize );
  }
  void AddPoint ( float x, float y )
  {
    if ( Data.size() < MaxSize )
      Data.push_back ( ImVec2 ( x,y ) );
    else
      {
        Data[Offset] = ImVec2 ( x,y );
        Offset = ( Offset + 1 ) % MaxSize;
      }
  }
  void Erase()
  {
    if ( Data.size() > 0 )
      {
        Data.shrink ( 0 );
        Offset  = 0;
      }
  }
};

// utility structure for realtime plot
struct RollingBuffer
{
  float Span;
  ImVector<ImVec2> Data;
  RollingBuffer()
  {
    Span = 10.0f;
    Data.reserve ( 2000 );
  }
  void AddPoint ( float x, float y )
  {
    float xmod = fmodf ( x, Span );
    if ( !Data.empty() && xmod < Data.back().x )
      Data.shrink ( 0 );
    Data.push_back ( ImVec2 ( xmod, y ) );
  }
};

void Demo_InfiniteLines()
{
  static double vals[] = {0.25, 0.5, 0.75};
  if ( ImPlot::BeginPlot ( "##Infinite" ) )
    {
      ImPlot::SetupAxes ( NULL,NULL,ImPlotAxisFlags_NoInitialFit,ImPlotAxisFlags_NoInitialFit );
      ImPlot::PlotInfLines ( "Vertical",vals,3 );
      ImPlot::PlotInfLines ( "Horizontal",vals,3,ImPlotInfLinesFlags_Horizontal );
      ImPlot::EndPlot();
    }
}

void Demo_Map ( TileManager& mngr, double lat, double lng)
{
  static int renders = 0;
  static bool debug = false;
  static double vals[] = {1, 1, 1};

  if ( debug )
    {
      int wk = mngr.threads_working();
      int dl = mngr.tiles_downloaded();
      int ld = mngr.tiles_loaded();
      int ca = mngr.tiles_cached();
      int fa = mngr.tiles_failed();
      ImGui::Text ( "FPS: %.2f    Working: %d    Downloads: %d    Loads: %d    Caches: %d    Fails: %d    Renders: %d", ImGui::GetIO().Framerate, wk, dl, ld, ca, fa, renders );
    }

  ImPlotAxisFlags ax_flags = ImPlotAxisFlags_NoLabel | ImPlotAxisFlags_NoTickLabels | ImPlotAxisFlags_NoGridLines| ImPlotAxisFlags_Foreground;
  if ( ImPlot::BeginPlot ( "##Map",ImVec2 ( -1,-1 ),ImPlotFlags_Equal|ImPlotFlags_NoMouseText ) )
    {
      ImPlot::SetupAxes ( NULL,NULL,ax_flags,ax_flags|ImPlotAxisFlags_Invert );
      ImPlot::SetupAxesLimits ( 0,1,0,1 );

        //Begin Overlay Plot
        ImPlot::PlotScatter ( "GPS Coords", &lat, &lng, 2);

        auto pos  = ImPlot::GetPlotPos();
      auto size = ImPlot::GetPlotSize();
      auto limits = ImPlot::GetPlotLimits();
      auto& region = mngr.get_region ( limits,size );
      renders = 0;
      if ( debug )
        {
          float ys[] = {1,1};
          ImPlot::SetNextFillStyle ( {1,0,0,1},0.5f );
          ImPlot::PlotShaded ( "##Bounds",ys,2 );
        }
      for ( auto& pair : region )
        {
          TileCoord coord            = pair.first;
          std::shared_ptr<Tile> tile = pair.second;
          auto [bmin,bmax] = coord.bounds();
          if ( tile != nullptr )
            {
              auto col = debug ? ( ( coord.x % 2 == 0 && coord.y % 2 != 0 ) || ( coord.x % 2 != 0 && coord.y % 2 == 0 ) ) ? ImVec4 ( 1,0,1,1 ) : ImVec4 ( 1,1,0,1 ) : ImVec4 ( 1,1,1,1 );
              ImPlot::PlotImage ( "##Tiles", ( void* ) ( intptr_t ) tile->image.ID,bmin,bmax, {0,0}, {1,1},col );
            }
        }
      ImPlot::PushPlotClipRect();
      static const char* label = "OpenStreetMap Contributors";
      auto label_size = ImGui::CalcTextSize ( label );
      auto label_off  = ImPlot::GetStyle().MousePosPadding;
      ImPlot::GetPlotDrawList()->AddText ( {pos.x + label_off.x, pos.y+size.y-label_size.y-label_off.y},IM_COL32_BLACK,label );
      ImPlot::PopPlotClipRect();


      ImPlot::EndPlot();
    }


}

void livePlot ( double vx, double vy, double vz, const char* name1, const char* name2, const char* name3)
{
  ImGui::BulletText ( "This is the Altitude Data" );
  static ScrollingBuffer vxdata1, vydata1, vzdata1;
  static RollingBuffer   vxdata2, vydata2, vzdata2;
  static float t = 0;
  t += ImGui::GetIO().DeltaTime;
  vxdata1.AddPoint ( t, vx * 0.01f );
  vydata1.AddPoint ( t, vy * 0.01f );
  vzdata1.AddPoint ( t, vz * 0.01f );

  vxdata2.AddPoint ( t, vx * 0.01f );
  vydata2.AddPoint ( t, vy * 0.01f );
  vzdata2.AddPoint ( t, vz * 0.01f );

  static float history = 10.0f;
  ImGui::SliderFloat ( "History",&history,1,120,"%.1f s" );
  vxdata2.Span = history;
  vydata2.Span = history;
  vzdata2.Span = history;


  if ( ImPlot::BeginPlot ( "##Scrolling", ImVec2 ( -1,150 ) ) )
    {
      ImPlot::SetupAxes ( NULL, NULL, NULL, NULL );
      ImPlot::SetupAxisLimits ( ImAxis_X1,t - history, t, ImGuiCond_Always );
      ImPlot::SetupAxisLimits ( ImAxis_Y1,0,1 );
      ImPlot::SetNextFillStyle ( IMPLOT_AUTO_COL,0.5f );
      ImPlot::PlotLine ( name1, &vxdata1.Data[0].x, &vxdata1.Data[0].y, vxdata1.Data.size(), 0, vxdata1.Offset, 2 * sizeof ( float ) );
      ImPlot::PlotLine ( name2, &vydata1.Data[0].x, &vydata1.Data[0].y, vydata1.Data.size(), 0, vydata1.Offset, 2*sizeof ( float ) );
      ImPlot::PlotLine ( name3, &vzdata1.Data[0].x, &vzdata1.Data[0].y, vzdata1.Data.size(), 0, vzdata1.Offset, 2*sizeof ( float ) );
      ImPlot::EndPlot();
    }
  if ( ImPlot::BeginPlot ( "##Rolling", ImVec2 ( -1,150 ) ) )
    {
      ImPlot::SetupAxes ( NULL, NULL, NULL, NULL );
      ImPlot::SetupAxisLimits ( ImAxis_X1,0,history, ImGuiCond_Always );
      ImPlot::SetupAxisLimits ( ImAxis_Y1,0,1 );
      ImPlot::PlotLine ( name1, &vxdata2.Data[0].x, &vxdata2.Data[0].y, vxdata2.Data.size(), 0, 0, 2 * sizeof ( float ) );
      ImPlot::PlotLine ( name2, &vydata2.Data[0].x, &vydata2.Data[0].y, vydata2.Data.size(), 0, 0, 2 * sizeof ( float ) );
      ImPlot::PlotLine ( name3, &vzdata2.Data[0].x, &vzdata2.Data[0].y, vzdata2.Data.size(), 0, 0, 2 * sizeof ( float ) );
      ImPlot::EndPlot();
    }
}
