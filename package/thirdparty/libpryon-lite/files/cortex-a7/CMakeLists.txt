cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
project(libpryon-lite)

INSTALL(FILES ./lib/libpryon_lite.a DESTINATION lib)
INSTALL(FILES ./models/D.en-US.alexa.bin DESTINATION models)
INSTALL(DIRECTORY include DESTINATION .)
