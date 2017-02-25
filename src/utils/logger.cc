#include "logger.hh"

namespace gdt {
log_recorder logger::recorder;
logger::severity logger::level = logger::DEBUG;
bool logger::check_level(logger::severity s) {
    return (s >= logger::level);
}

std::string logger::ts() {
}



}
