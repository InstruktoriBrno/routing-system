cmake_minimum_required(VERSION 3.14)

add_executable(rg_draw)
target_sources(rg_draw PRIVATE draw.cpp)
target_link_libraries(rg_draw PRIVATE rg::io)

