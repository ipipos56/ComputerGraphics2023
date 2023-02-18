#define TINYOBJLOADER_IMPLEMENTATION

#include "model.h"

#include "utils/error_handler.h"

#include <linalg.h>


using namespace linalg::aliases;
using namespace cg::world;

cg::world::model::model() {}

cg::world::model::~model() {}

void cg::world::model::load_obj(const std::filesystem::path& model_path)
{
	tinyobj::ObjReaderConfig readerConfig;
	readerConfig.mtl_search_path = model_path.parent_path().string();// Path to material files
	readerConfig.triangulate = true;

	tinyobj::ObjReader reader;
	if (!reader.ParseFromFile(model_path.string(), readerConfig))
	{
		if (!reader.Error().empty())
			THROW_ERROR(reader.Error());
	}

	auto& shapes = reader.GetShapes();
	auto& attrib = reader.GetAttrib();
	auto& materials = reader.GetMaterials();

	allocate_buffers(shapes);
	fill_buffers(shapes, attrib, materials, model_path.parent_path());
}

void model::allocate_buffers(const std::vector<tinyobj::shape_t>& shapes)
{
	for (const auto& shape: shapes)
	{
		size_t index_offset = 0;
		unsigned int vertex_buffer_size = 0;
		unsigned int index_buffer_size = 0;
		std::map<std::tuple<int, int, int>, unsigned int> index_map;
		const auto& mesh = shape.mesh;

		for (const auto& fv: mesh.num_face_vertices)
		{
			for (size_t v = 0; v < fv; v++)
			{
				tinyobj::index_t idx = mesh.indices[index_offset + v];
				auto idx_tuple = std::make_tuple(idx.vertex_index, idx.normal_index, idx.texcoord_index);
				if (index_map.count(idx_tuple) == 0)
				{
					index_map[idx_tuple] = vertex_buffer_size;
					vertex_buffer_size++;
				}
				index_buffer_size++;
			}
			index_offset += fv;
		}
		vertex_buffers.push_back(std::make_shared<cg::resource<cg::vertex>>(vertex_buffer_size));
		index_buffers.push_back(std::make_shared<cg::resource<unsigned int>>(index_buffer_size));
	}
	textures.resize(shapes.size());
}

float3 cg::world::model::compute_normal(const tinyobj::attrib_t& attrib, const tinyobj::mesh_t& mesh, size_t index_offset)
{
	auto a_id = mesh.indices[index_offset].vertex_index;
	auto b_id = mesh.indices[index_offset + 1].vertex_index;
	auto c_id = mesh.indices[index_offset + 2].vertex_index;

	auto a = float3{attrib.vertices[3 * a_id], attrib.vertices[3 * a_id + 1], attrib.vertices[3 * a_id + 2]};
	auto b = float3{attrib.vertices[3 * b_id], attrib.vertices[3 * b_id + 1], attrib.vertices[3 * b_id + 2]};
	auto c = float3{attrib.vertices[3 * c_id], attrib.vertices[3 * c_id + 1], attrib.vertices[3 * c_id + 2]};

	auto ab = b - a;
	auto ac = c - a;

	auto normal = cross(ab, ac);
	normal = normalize(normal);

	return normal;
}

void model::fill_vertex_data(cg::vertex& vertex, const tinyobj::attrib_t& attrib, const tinyobj::index_t idx, const float3 computed_normal, const tinyobj::material_t material)
{
	vertex.x = attrib.vertices[3 * idx.vertex_index];
	vertex.y = attrib.vertices[3 * idx.vertex_index + 1];
	vertex.z = attrib.vertices[3 * idx.vertex_index + 2];

	if (idx.normal_index < 0)
	{
		vertex.nx = computed_normal.x;
		vertex.ny = computed_normal.y;
		vertex.nz = computed_normal.z;
	}
	else
	{
		vertex.nx = attrib.normals[3 * idx.normal_index];
		vertex.ny = attrib.normals[3 * idx.normal_index + 1];
		vertex.nz = attrib.normals[3 * idx.normal_index + 2];
	}
}

void model::fill_buffers(const std::vector<tinyobj::shape_t>& shapes, const tinyobj::attrib_t& attrib, const std::vector<tinyobj::material_t>& materials, const std::filesystem::path& base_folder)
{
	// TODO Lab: 1.03 Using `tinyobjloader` implement `load_obj`, `allocate_buffers`, `compute_normal`, `fill_vertex_data`, `fill_buffers`, `get_vertex_buffers`, `get_index_buffers` methods of `cg::world::model` class
}


const std::vector<std::shared_ptr<cg::resource<cg::vertex>>>&
cg::world::model::get_vertex_buffers() const
{
	// TODO Lab: 1.03 Using `tinyobjloader` implement `load_obj`, `allocate_buffers`, `compute_normal`, `fill_vertex_data`, `fill_buffers`, `get_vertex_buffers`, `get_index_buffers` methods of `cg::world::model` class
	return vertex_buffers;
}

const std::vector<std::shared_ptr<cg::resource<unsigned int>>>&
cg::world::model::get_index_buffers() const
{
	// TODO Lab: 1.03 Using `tinyobjloader` implement `load_obj`, `allocate_buffers`, `compute_normal`, `fill_vertex_data`, `fill_buffers`, `get_vertex_buffers`, `get_index_buffers` methods of `cg::world::model` class
	return index_buffers;
}

const std::vector<std::filesystem::path>& cg::world::model::get_per_shape_texture_files() const
{
	return textures;
}


const float4x4 cg::world::model::get_world_matrix() const
{
	return float4x4{
			{1, 0, 0, 0},
			{0, 1, 0, 0},
			{0, 0, 1, 0},
			{0, 0, 0, 1}};
}
