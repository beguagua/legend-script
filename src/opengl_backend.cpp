#include "opengl_backend.hpp"

#include <stdexcept>

namespace legend::graphics {

std::string OpenGLVersion::name() const {
  return "OpenGL " + std::to_string(major) + "." + std::to_string(minor) +
         (core_profile ? " Core" : " Compatibility");
}

bool OpenGLVersion::is_supported_by_legend() const {
  return OpenGLBackend::supports(major, minor);
}

bool OpenGLBackend::supports(int major, int minor) {
  // Legend accepts every desktop OpenGL release from 1.0 through 4.6.
  // The actual driver may expose a smaller range; the host checks that when
  // it creates the native context.
  return (major == 1 && minor >= 0 && minor <= 5) ||
         (major == 2 && minor >= 0 && minor <= 1) ||
         (major == 3 && minor >= 0 && minor <= 3) ||
         (major == 4 && minor >= 0 && minor <= 6);
}

const char* OpenGLBackend::profile_name(bool core) {
  return core ? "core" : "compatibility";
}

OpenGLVersion OpenGLBackend::negotiate(int major, int minor, bool core,
                                       bool debug) {
  if (!supports(major, minor)) {
    throw std::invalid_argument("Legend supports OpenGL desktop versions 1.0 through 4.6");
  }
  // Core profiles were introduced by OpenGL 3.2. Older versions use the
  // compatibility profile semantics even if the caller requested core.
  if (core && (major < 3 || (major == 3 && minor < 2))) {
    throw std::invalid_argument("OpenGL core profile requires OpenGL 3.2 or newer");
  }
  return OpenGLVersion{major, minor, core, debug};
}

}  // namespace legend::graphics
