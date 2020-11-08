// Copyright © 2020 Luigi Rapetta. All Rights Reserved.

#ifndef FCPP_GRAPHICS_RENDERER_H_
#define FCPP_GRAPHICS_RENDERER_H_

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb_image/stb_image.h>

#include "lib/graphics/camera.h"
#include "lib/graphics/shader.h"
#include "lib/graphics/shapes.h"


namespace fcpp {
	namespace internal {
		class Renderer {
		private:
			// put stuff here, b0i

		public:
			//!@brief Renderer constructor, with GLFW and OpenGL initializations.
			Renderer();
		};
	}
}

#endif // FCPP_GRAPHICS_RENDERER_H_