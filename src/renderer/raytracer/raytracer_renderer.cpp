#include "raytracer_renderer.h"

#include "utils/resource_utils.h"

#include <iostream>


void cg::renderer::ray_tracing_renderer::init()
{
	render_target = std::make_shared<cg::resource<cg::unsigned_color>>(
			settings->width, settings->height);

	model = std::make_shared<cg::world::model>();
	model->load_obj(settings->model_path);

	camera = std::make_shared<cg::world::camera>();
	camera->set_height(static_cast<float>(settings->height));
	camera->set_width(static_cast<float>(settings->width));
	camera->set_position(float3(settings->camera_position[0],
								settings->camera_position[1],
								settings->camera_position[2]));

	camera->set_phi(settings->camera_phi);
	camera->set_theta(settings->camera_theta);
	camera->set_angle_of_view(settings->camera_angle_of_view);
	camera->set_z_near(settings->camera_z_near);
	camera->set_z_far(settings->camera_z_far);

	raytracer = std::make_shared<cg::renderer::raytracer<cg::vertex, cg::unsigned_color>>();
	raytracer->set_viewport(settings->width, settings->height);
	raytracer->set_render_target(render_target);

	raytracer->set_vertex_buffers(model->get_vertex_buffers());
	raytracer->set_index_buffers(model->get_index_buffers());

	lights.push_back({
			float3{-0.24f, 1.8f, 0.16f},
			float3{0.78f, 0.78f, 0.78f} * 0.25f
	});
	lights.push_back({
			float3{-0.24f, 1.8f, -0.22f},
			float3{0.78f, 0.78f, 0.78f} * 0.25f
	});
	lights.push_back({
			float3{0.23f, 1.8f, 0.16f},
			float3{0.78f, 0.78f, 0.78f} * 0.25f
	});
	lights.push_back({
			float3{0.23f, 1.8f, -0.22f},
			float3{0.78f, 0.78f, 0.78f} * 0.25f
	});

	shadow_raytracer = std::make_shared<cg::renderer::raytracer<cg::vertex, cg::unsigned_color>>();

}

void cg::renderer::ray_tracing_renderer::destroy() {}

void cg::renderer::ray_tracing_renderer::update() {}

void cg::renderer::ray_tracing_renderer::render()
{
	shadow_raytracer->miss_shader = [](const ray& ray) {
		payload payload{};
		payload.t = -1.f;
		return payload;
	};

	shadow_raytracer->any_hit_shader = [](const ray& ray, payload& payload, const triangle<cg::vertex>& triangle) {
		return payload;
	};

	raytracer->clear_render_target({0, 0, 0});

	raytracer->miss_shader = [](const ray& ray) {
		payload payload{};
		payload.color = {0.f, 0.f, 0.f};
		return payload;
	};

	std::random_device randomDevice;
	std::mt19937 randomGenerator(randomDevice());
	std::uniform_real_distribution<float> uniformDistribution(-1.f, 1.f);

	raytracer->closest_hit_shader = [&](const ray& ray, payload& payload, const triangle<cg::vertex>& triangle, size_t depth) {
		float3 position = ray.position + ray.direction * payload.t;
		float3 normal = normalize(payload.bary.x * triangle.na + payload.bary.y * triangle.nb + payload.bary.z * triangle.nc);
		float3 result_color = triangle.emissive;

		float3 random_direction = normalize(float3(uniformDistribution(randomGenerator),
												  uniformDistribution(randomGenerator),
												  uniformDistribution(randomGenerator)));

		if(dot(normal, random_direction) < 0.f)
		{
			random_direction = -random_direction;
		}

		cg::renderer::ray to_next_object(position, random_direction);
		auto payload_next = raytracer->trace_ray(to_next_object, depth);

		result_color += triangle.diffuse * payload_next.color.to_float3() * std::max(dot(normal, to_next_object.direction), 0.f);

		payload.color = cg::color::from_float3(result_color);
		return payload;
	};

	raytracer->build_acceleration_structure();
	shadow_raytracer->acceleration_structures = raytracer->acceleration_structures;


	auto start = std::chrono::high_resolution_clock::now();
	raytracer->ray_generation(camera->get_position(), camera->get_direction(),
							  camera->get_right(), camera->get_up(), settings->raytracing_depth, settings->accumulation_num);

	auto stop = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
	std::cout << "Raytracing time: " << duration.count() << "ms" << std::endl;


	cg::utils::save_resource(*render_target, settings->result_path);

}