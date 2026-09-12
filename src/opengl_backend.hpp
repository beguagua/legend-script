#pragma once

#include <string>

namespace legend::graphics {

struct OpenGLVersion {
  int major = 1;
  int minor = 0;
  bool core_profile = false;
  bool debug_context = false;

  [[nodiscard]] std::string name() const;
  [[nodiscard]] bool is_supported_by_legend() const;
};

class OpenGLBackend {
 public:
  // The request is validated without creating a window/context. A platform
  // host can later use this description to create WGL/GLX/EGL/CGL contexts.
  static OpenGLVersion negotiate(int major, int minor, bool core = false,
                                 bool debug = false);
  static bool supports(int major, int minor);
  static const char* profile_name(bool core);
};

}  // namespace legend::graphics
