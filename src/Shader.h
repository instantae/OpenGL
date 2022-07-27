#pragma once
#include <string>
#include <unordered_map>
#include <glad.h>
#include <glm/glm.hpp>

struct ShaderSource
{
	std::string VertexSource;
	std::string FragmentSource;
};

class Shader
{
private:
	std::string m_FilePath;
	
	std::unordered_map<std::string, GLint> m_UniformLocationCache;

public:
	Shader(const std::string& filepath);
	~Shader();

	GLuint m_RendererID;

	void Bind() const;
	void Unbind() const;

	// Set uniforms
	void SetUniform1i(const std::string& name, int value);
	void SetUniform1f(const std::string& name, float value);
	void SetUniform3f(const std::string& name, const glm::vec3& vec3);
	void SetUniform4f(const std::string& name, const glm::vec4& vec4);
	
	void SetUniformMat4f(const std::string& name, const glm::mat4& mat);

private:
	ShaderSource ParseShader(const std::string& filepath);
	GLuint CompileShader(const std::string& source, GLenum type);
	GLuint CreateShader(const std::string& vertexShader, const std::string& fragmentShader);

	GLint GetUniformLocation(const std::string& name);
};