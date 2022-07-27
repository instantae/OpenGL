#include "VertexArray.h"
#include "VertexBufferLayout.h"
#include "renderer.h"

VertexArray::VertexArray()
{
	GLCall(glGenVertexArrays(1, &m_RendererID));
}

VertexArray::~VertexArray()
{
	GLCall(glDeleteVertexArrays(1, &m_RendererID));
}

void VertexArray::AddBuffer(const VertexBuffer& buffer, const VertexBufferLayout& layout) const
{
	Bind();
	buffer.Bind();
	const auto& elements = layout.GetElements();
	GLuint offset = 0;
	for (GLuint i = 0; i < elements.size(); i++)
	{
		const auto& element = elements[i];
		GLCall(glEnableVertexAttribArray(i));
		GLCall(glVertexAttribPointer(i, element.count, element.type, element.normalized, layout.GetStride(), (const void*)offset));
		offset += element.count * VertexBufferElement::GetSizeOfType(element.type);
	}


}

void VertexArray::Bind() const
{
	GLCall(glBindVertexArray(m_RendererID));

}

void VertexArray::Unbind() const
{
	GLCall(glBindVertexArray(0));

}
