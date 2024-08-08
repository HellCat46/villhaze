cmake_minimum_required(VERSION 3.28)
project(villhaze CXX)

find_package(OpenSSL REQUIRED)

set(CMAKE_XXX_STANDARD 11)

add_executable(villhaze
		src/Encoder/base64.h
		src/Parsers/HTTPRequest.h
		src/WebSocket.h
		src/main.cpp
)

target_link_libraries(villhaze OpenSSL::SSL OpenSSL::Crypto)
