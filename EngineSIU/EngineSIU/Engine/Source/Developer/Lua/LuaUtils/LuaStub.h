#include <sol/sol.hpp>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <assert.h>


// Stub 파일 생성 함수
void generateStubs(sol::state& lua) {
    // TODO 일단은 Stub 파일 전부 만듬
    // 이후에 시간 남으면 아래 함수 오류 고칠것
    // sol::table idx = mt["__index"]; 이쪽이 문제임
    return;
    // 1) SIUEngine 테이블 얻기
    sol::table ns = lua["SIUEngine"];
    assert(ns.valid());
    if (!ns.valid()) {
        std::cerr << "Error: SIUEngine namespace not found" << std::endl;
        return;
    }

    // 2) stub 디렉터리 생성
    std::filesystem::create_directories("lua_stubs");
    std::ofstream file("lua_stubs/SIUEngine.lua");
    assert(file);
    if (!file) {
        std::cerr << "Error: cannot open lua_stubs/SIUEngine.lua for writing" << std::endl;
        return;
    }

    // 3) 파일 헤더 작성x
    file << "-- lua_stubs/SIUEngine.lua\n\n";
    file << "---@module SIUEngine\n\n";

    // 4) 디버그 라이브러리 로드 및 getmetatable 함수
    lua.open_libraries(sol::lib::debug);
    sol::function getmt = lua["debug"]["getmetatable"];

    // 5) 네임스페이스 순회: 모듈 필드 및 클래스별 stub 작성

// 5-1) 모듈 레벨 필드(함수·변수) 스텁
    for (auto& kv : ns) {
        const std::string key = kv.first.as<std::string>();
        sol::object val = kv.second;
        sol::type t = val.get_type();
        if (t == sol::type::table) continue;  // 클래스는 이후 처리
        // 함수
        if (t == sol::type::function) {
            file << "---@field " << key << " fun(...)\n";
        }
        // 숫자
        else if (t == sol::type::number) {
            file << "---@field " << key << " number\n";
        }
        // 문자열
        else if (t == sol::type::string) {
            file << "---@field " << key << " strin\n";
        }
        // 불리언
        else if (t == sol::type::boolean) {
            file << "---@field " << key << " boolean\n";
        }
        // 기타(Userdata 등)
        else {
            file << "---@field " << key << " any\n";
        }
    }
    file << "\n";

        // 5-2) 클래스별 stub
        for (auto& kv : ns) {
            std::string class_name = kv.first.as<std::string>();
            sol::object val = kv.second;
            if (val.get_type() != sol::type::table) continue;

            // 클래스 선언
            file << "---@class SIUEngine." << class_name << "\n";
                // 생성자 stub
                file << "---@field new fun(...):SIUEngine." << class_name << "\n";

                // 메서드 목록 추출
                sol::table class_tbl = ns[class_name];
            sol::table mt = getmt(class_tbl);
            if (mt.valid()) {
                sol::table idx = mt["__index"];
                for (auto& mkv : idx) {
                    std::string mname = mkv.first.as<std::string>();
                    file << "---@field " << mname
                        << " fun(self:SIUEngine." << class_name << ")\n";
                }
            }
            file << "\n";
        }

    // 6) 네임스페이스 리턴
    file << "local SIUEngine = {}\n";
        file << "return SIUEngine\n";
        file.close();
    std::cout << "Generated lua_stubs/SIUEngine.lua" << std::endl;
}
