#ifndef GDT_SCREEN_HEADER_INCLUDED
#define GDT_SCREEN_HEADER_INCLUDED

#include <vector>

namespace gdt {

struct screen {  // screen size structure
    int w;
    int h;
    class subscriber {
      public:
        virtual void on_screen_resize(unsigned int width,
                                      unsigned int height) = 0;
    };
    void notify_subscribers() {
        for (auto s : _subscribers) {
            s->on_screen_resize(w, h);
        }
    }
    void subscribe(subscriber *s) {
        _subscribers.push_back(s);
        s->on_screen_resize(w, h);
    }
    std::vector<subscriber *> _subscribers;
};

}
#endif  // GDT_SCREEN_HEADER_INCLUDED
