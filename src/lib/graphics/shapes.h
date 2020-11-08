// Copyright © 2020 Luigi Rapetta. All Rights Reserved.

#ifndef FCPP_GRAPHICS_SHAPES_H_
#define FCPP_GRAPHICS_SHAPES_H_

namespace fcpp {

	//! @brief Supported shapes for representing nodes.
	enum class shape { ortho, cube, sphere };

	namespace internal {
		class Shapes {
        public:
			//! @brief Cube's vertex data.
			static const float VERTEX_CUBE[216];

			//! @brief Orthogonal axis' vertex data.
			static const float VERTEX_ORTHO[144];

			//! @brief Orthogonal axis' index data.
			static const int INDEX_ORTHO[108];

			/*
            //Get the vertex data of the specified shape.
            static const float* getVertex(shape sh);

            //Get the index data of the specified shape.
            static const int* getIndex(shape sh);
			*/
		};
	}
}

#endif // FCPP_GRAPHICS_SHAPES_H_