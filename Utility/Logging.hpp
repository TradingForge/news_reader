#pragma once

#ifndef __LOGS_DIRECTORY_PATH
    #error LOGS DIRECTORY PATH NOT SPECIFIED
#endif
#ifndef __TEST_LOGS_DIRECTORY_PATH
    #error TEST LOGS DIRECTORY PATH NOT SPECIFIED
#endif

#include <bits/stdc++.h>

#include <P7/P7_Trace.h>
#include <P7/P7_Telemetry.h>
#include <P7/P7_Client.h>

#define LOG(VERBOSITY, MESSAGE, ...) { using namespace Logging; using namespace IP7_Targets; Logger::IP7_STDOUT_TRACE->P7_##VERBOSITY(Logger::IP7_MAIN_MODULE, MESSAGE __VA_OPT__(,) __VA_ARGS__); Logger::IP7_FILE_TRACE->P7_##VERBOSITY(Logger::IP7_MAIN_MODULE, MESSAGE __VA_OPT__(,) __VA_ARGS__); Logger::IP7_VERBOSE_TRACE->P7_##VERBOSITY(Logger::IP7_VERBOSE_MODULE, MESSAGE __VA_OPT__(,) __VA_ARGS__); }
#define LOGT(TARGET, VERBOSITY, MESSAGE, ...) { using namespace Logging; using namespace IP7_Targets; assert(TARGET == StdOut || TARGET == File || TARGET == All); if (TARGET == StdOut || TARGET == All) { Logger::IP7_STDOUT_TRACE->P7_##VERBOSITY(Logger::IP7_MAIN_MODULE, MESSAGE __VA_OPT__(,) __VA_ARGS__); } if (TARGET == File || TARGET == All) { Logger::IP7_FILE_TRACE->P7_##VERBOSITY(Logger::IP7_MAIN_MODULE, MESSAGE __VA_OPT__(,) __VA_ARGS__); } if (TARGET == Verbose || TARGET == All) { Logger::IP7_VERBOSE_TRACE->P7_##VERBOSITY(Logger::IP7_VERBOSE_MODULE, MESSAGE __VA_OPT__(,) __VA_ARGS__); } }
#define LOGM(MODULE, VERBOSITY, MESSAGE, ...) { using namespace Logging; using namespace IP7_Targets; Logger::IP7_STDOUT_TRACE->P7_##VERBOSITY(MODULE, MESSAGE __VA_OPT__(,) __VA_ARGS__); Logger::IP7_FILE_TRACE->P7_##VERBOSITY(MODULE, MESSAGE __VA_OPT__(,) __VA_ARGS__); Logger::IP7_VERBOSE_TRACE->P7_##VERBOSITY(MODULE, MESSAGE __VA_OPT__(,) __VA_ARGS__); }
#define LOGMT(MODULE, TARGET, VERBOSITY, MESSAGE, ...) { using namespace Logging; using namespace IP7_Targets; assert(TARGET == StdOut || TARGET == File || TARGET == All); if (TARGET == StdOut || TARGET == All) { Logger::IP7_STDOUT_TRACE->P7_##VERBOSITY(MODULE, MESSAGE __VA_OPT__(,) __VA_ARGS__); } if (TARGET == File || TARGET == All) { Logger::IP7_FILE_TRACE->P7_##VERBOSITY(MODULE, MESSAGE __VA_OPT__(,) __VA_ARGS__); } if (TARGET == Verbose || TARGET == All) { Logger::IP7_VERBOSE_TRACE->P7_##VERBOSITY(MODULE, MESSAGE __VA_OPT__(,) __VA_ARGS__); }  }

#undef ERROR

namespace Logging {
    
    enum {
        TRACE,
        DEBUG,
        INFO,
        WARNING,
        ERROR,
        CRITICAL
    };
    
    class Logger final {

    private:

        using string = std::string;
        using runtime_error = std::runtime_error;

        static constexpr const tXCHAR *IP7_MAIN_MODULE_NAME = TM("IP7_MAIN");
        static constexpr const tXCHAR *IP7_VERBOSE_MODULE_NAME = TM("IP7_VERBOSE");

        static IP7_Client* IP7_VERBOSE_CLIENT;
        static IP7_Client* IP7_FILE_CLIENT;
        static IP7_Client* IP7_STDOUT_CLIENT;

        static std::mutex _mutex;

        Logger() = default;

    public:

        static IP7_Trace::hModule IP7_MAIN_MODULE;
        static IP7_Trace::hModule IP7_VERBOSE_MODULE;

        static IP7_Trace *IP7_VERBOSE_TRACE;
        static IP7_Trace *IP7_FILE_TRACE;
        static IP7_Trace *IP7_STDOUT_TRACE;

        static void Initialize(bool test) {

            if (!_mutex.try_lock()) {
                throw runtime_error("ATTEMPT TO INITIALIZE LOGGER TWICE");
            }
            
            P7_Set_Crash_Handler();

            const auto IP7_VERBOSE_CLIENT_ARGS = TM("/P7.Sink=FileTxt /P7.Dir=") TM(__LOGS_DIRECTORY_PATH) TM("/verbose /P7.Trc.Verb=0 /P7.Format=\"%lv (%tf) [%ti] {%fs:%fl:%fn} > %ms\"");
            const auto TEST_IP7_VERBOSE_CLIENT_ARGS = TM("/P7.Sink=FileTxt /P7.Dir=") TM(__TEST_LOGS_DIRECTORY_PATH) TM("/verbose /P7.Trc.Verb=0 /P7.Format=\"%lv (%tf) [%ti] {%fs:%fl:%fn} > %ms\"");
            IP7_VERBOSE_CLIENT = P7_Create_Client(test ? TEST_IP7_VERBOSE_CLIENT_ARGS : IP7_VERBOSE_CLIENT_ARGS);
            if (!IP7_VERBOSE_CLIENT) {
                throw runtime_error("could not Create a P7 verbose client");
            }
            IP7_VERBOSE_TRACE = P7_Create_Trace(IP7_VERBOSE_CLIENT, TM("IP7_VERBOSE_TRACE"));
            if (!IP7_VERBOSE_TRACE) {
                throw runtime_error("could not Create a P7 verbose Trace");
            }
            if (!IP7_VERBOSE_TRACE->Register_Module(IP7_VERBOSE_MODULE_NAME, &IP7_VERBOSE_MODULE)) {
                throw runtime_error("could not register a P7 verbose Trace module");
            }
            if (!IP7_VERBOSE_TRACE->Register_Thread(IP7_VERBOSE_MODULE_NAME, 0)) {
                throw runtime_error("could not register a P7 verbose Trace module");
            }

            const auto IP7_FILE_CLIENT_ARGS = TM("/P7.Sink=FileTxt /P7.Dir=") TM(__LOGS_DIRECTORY_PATH) TM(" /P7.Trc.Verb=1 /P7.Format=\"%tf | %lv | %ms\"");
            const auto TEST_IP7_FILE_CLIENT_ARGS = TM("/P7.Sink=FileTxt /P7.Dir=") TM(__TEST_LOGS_DIRECTORY_PATH) TM(" /P7.Trc.Verb=1 /P7.Format=\"%tf | %lv | %ms\"");
            IP7_FILE_CLIENT = P7_Create_Client(test ? TEST_IP7_FILE_CLIENT_ARGS : IP7_FILE_CLIENT_ARGS);
            if (!IP7_FILE_CLIENT) {
                throw runtime_error("could not Create a P7 file client");
            }
            IP7_FILE_TRACE = P7_Create_Trace(IP7_FILE_CLIENT, TM("IP7_FILE_TRACE"));
            if (!IP7_FILE_TRACE) {
                throw runtime_error("could not Create a P7 file Trace");
            }
            if (!IP7_FILE_TRACE->Register_Module(IP7_MAIN_MODULE_NAME, &IP7_MAIN_MODULE)) {
                throw runtime_error("could not register a P7 file Trace module");
            }
            if (!IP7_FILE_TRACE->Register_Thread(IP7_MAIN_MODULE_NAME, 0)) {
                throw runtime_error("could not register a P7 file Trace module");
            }

            IP7_STDOUT_CLIENT = P7_Create_Client(TM("/P7.Sink=Console /P7.Trc.Verb=2 /P7.Format=\"%tf | %lv | %ms\""));
            if (!IP7_STDOUT_CLIENT) {
                throw runtime_error("could not Create a P7 stdout client");
            }
            IP7_STDOUT_TRACE = P7_Create_Trace(IP7_STDOUT_CLIENT, TM("IP7_STDOUT_TRACE"));
            if (!IP7_STDOUT_TRACE) {
                throw runtime_error("could not Create a P7 stdout Trace");
            }
            if (!IP7_STDOUT_TRACE->Register_Module(IP7_MAIN_MODULE_NAME, &IP7_MAIN_MODULE)) {
                throw runtime_error("could not register a P7 stdout Trace module");
            }
            if (!IP7_STDOUT_TRACE->Register_Thread(IP7_MAIN_MODULE_NAME, 0)) {
                throw runtime_error("could not register a P7 stdout Trace module");
            }

        }

        enum class IP7_Target {
            StdOut = 1,
            File = 2,
            Verbose = 4,
            All = 7
        };

    };

    std::mutex Logger::_mutex = std::mutex();

    IP7_Client* Logger::IP7_VERBOSE_CLIENT = nullptr;
    IP7_Client* Logger::IP7_FILE_CLIENT = nullptr;
    IP7_Client* Logger::IP7_STDOUT_CLIENT = nullptr;

    IP7_Trace::hModule Logger::IP7_MAIN_MODULE = nullptr;
    IP7_Trace::hModule Logger::IP7_VERBOSE_MODULE = nullptr;

    IP7_Trace *Logger::IP7_VERBOSE_TRACE = nullptr;
    IP7_Trace *Logger::IP7_FILE_TRACE = nullptr;
    IP7_Trace *Logger::IP7_STDOUT_TRACE = nullptr;
    
    namespace IP7_Targets {
        constexpr Logger::IP7_Target StdOut = Logger::IP7_Target::StdOut;
        constexpr Logger::IP7_Target File = Logger::IP7_Target::File;
        constexpr Logger::IP7_Target Verbose = Logger::IP7_Target::Verbose;
        constexpr Logger::IP7_Target All = Logger::IP7_Target::All;
    }

}
