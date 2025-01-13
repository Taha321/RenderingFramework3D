#include <memory>
#include <vector>
//#include "renderer.h"
#include "renderer_internal.h"
#include "wnd_internal.h"

namespace RenderingFramework3D {

using namespace MathUtil;

Renderer::Renderer() {
	_internal = std::make_unique<RendererInternal>();
}

Renderer::~Renderer() {}

bool Renderer::Initialize(Window& wnd) {
	return _internal->Initialize(wnd._internal);
}

bool Renderer::Cleanup() {
	return _internal->Cleanup();
}

void Renderer::SetLightDirection(const MathUtil::Vec<3>& direction) {
	_internal->SetLightDirection(direction);
}
void Renderer::SetLightDirection(unsigned pipeline, const MathUtil::Vec<3>& direction) {
	_internal->SetLightDirection(pipeline, direction);
}
void Renderer::SetLightDirection(const std::vector<unsigned>& pipelines, const Vec<3>& direction) {
	for(unsigned p : pipelines) {
		_internal->SetLightDirection(p, direction);
	}
}

void Renderer::SetLightColour(const MathUtil::Vec<4>& colour) {
	_internal->SetLightColour(colour);
}
void Renderer::SetLightColour(unsigned pipeline, const MathUtil::Vec<4>& colour) {
	_internal->SetLightColour(pipeline, colour);
}
void Renderer::SetLightColour(const std::vector<unsigned>& pipelines, const MathUtil::Vec<4>& colour) {
	for(unsigned p : pipelines) {
		_internal->SetLightColour(p, colour);
	}
}

void Renderer::SetLightIntensity(float intensity) {
	_internal->SetLightIntensity(intensity);
}
void Renderer::SetLightIntensity(unsigned pipeline, float intensity) {
	_internal->SetLightIntensity(pipeline, intensity);
}
void Renderer::SetLightIntensity(const std::vector<unsigned>& pipelines, float intensity) {
	for(unsigned p : pipelines) {
		_internal->SetLightIntensity(p, intensity);
	}
}

void Renderer::SetAmbientLightIntensity(float intensity) {
	_internal->SetAmbientLightIntensity(intensity);
}
void Renderer::SetAmbientLightIntensity(unsigned pipeline, float intensity) {
	_internal->SetAmbientLightIntensity(pipeline, intensity);
}
void Renderer::SetAmbientLightIntensity(const std::vector<unsigned>& pipelines, float intensity) {
	for(unsigned p : pipelines) {
		_internal->SetAmbientLightIntensity(p, intensity);
	}
}

void Renderer::SetCustomGlobalUniformShaderData(unsigned pipeline, unsigned binding, void* data, unsigned size, unsigned offset) {
	_internal->SetCustomGlobalUniformShaderData(pipeline, binding, data, size, offset);
}

void Renderer::SetCustomGlobalUniformShaderData(const std::vector<unsigned>& pipelines, unsigned binding, void* data, unsigned size, unsigned offset) {
	for(unsigned p : pipelines) {
		_internal->SetCustomGlobalUniformShaderData(p, binding, data, size, offset);
	}
}

bool Renderer::DrawObject(const WorldObject& obj, Camera& cam, unsigned pipelineID) {
	return _internal->DrawObject(obj, cam, pipelineID);
}

bool Renderer::PresentFrame() {
	return _internal->PresentFrame();
}


bool Renderer::CreateCustomPipeline(const PipelineConfig& config, unsigned& pipelineID) {
	if(_internal->IsReady()) {
		return _internal->CreatePipeline(config, pipelineID);
	}
	return false;
}
}