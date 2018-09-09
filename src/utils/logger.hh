#ifndef SRC_UTILS_LOGGER_HH_INCLUDED
#define SRC_UTILS_LOGGER_HH_INCLUDED

#include <iostream>
#include <sstream>
#include <chrono>
#define UNUSED(x) (void)(x)

namespace gdt {
    class log_record;
    class log_recorder {
        public:
            void operator <<(log_record&& t) {
                UNUSED(t);
            }
    };

    /**
     * GDT provides a stream logger you can use just like you would use
     * std::cout, with an added severity filtering functionality.
     *
     * You don't normally use the logger class directly. Instead, use
     * the predefined macros (yes, unfortunately macros):
     *
     *     LOG_DEBUG << "your debug message " << "goes here..";
     *     LOG_INFO << "your info message " << "goes here..";
     *     LOG_WARNING << "your warning message " << "goes here..";
     *     LOG_ERROR << "your error message " << "goes here..";
     *
     *
     */
    class logger {
        public:
            enum severity {
                DEBUG = 0,
                INFO,
                WARNING,
                ERROR
            };

            static log_recorder recorder;
            static severity level;
            static bool check_level(severity s);
    };

    class log_record {
        private:
            std::stringstream _stream;
        public:
            log_record() {
                std::cout << std::chrono::high_resolution_clock::now().time_since_epoch().count() << " ";
            }
            virtual ~log_record() {
                std::cout << str() << std::endl;
            }
            operator std::stringstream&() {
                return _stream;
            }
            template <typename T>
            std::stringstream & operator <<(T&& t) {
                _stream << t;
                return _stream;
            }
            std::string str() const { return _stream.str(); }


    };


}

//#define IF_LOG_(SEVERITY) \
//    !gdt::logger::check_level(SEVERITY) ? void(0) :
#ifdef DEBUG_LOGS
#define LOG_DEBUG \
    gdt::log_record() << "[DEBUG] (" << __FILE__ << ":" << __LINE__ << ") "
#else
#define LOG_DEBUG  if (0) gdt::log_record()
#endif

#ifdef INFO_LOGS
#define LOG_INFO \
    gdt::log_record() << "[INFO] (" << __FILE__ << ":" << __LINE__ << ") "
#else
#define LOG_INFO  if (0) gdt::log_record()
#endif

#ifdef WARNING_LOGS
#define LOG_WARNING \
    gdt::log_record() << "[WARNING] (" << __FILE__ << ":" << __LINE__ << ") "
#else
#define LOG_WARNING  if (0) gdt::log_record()
#endif

#ifdef ERROR_LOGS
#define LOG_ERROR \
    gdt::log_record() << "[ERROR] (" << __FILE__ << ":" << __LINE__ << ") "
#else
#define LOG_ERROR  if (0) gdt::log_record()
#endif


//    IF_LOG_(gdt::logger::INFO) gdt::log_record() << gdt::logger::ts() << "[INFO] (" << __FILE__ << ":" << __LINE__ << ") "

#endif  // SRC_UTILS_LOGGER_HH_INCLUDED
