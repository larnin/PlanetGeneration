//#include "Render.h"

#include "Utilities.h"
#include <Nazara/Utility/IndexBuffer.hpp>
#include <Nazara/Utility/VertexBuffer.hpp>
#include <Nazara/Utility/VertexMapper.hpp>
#include <Nazara/Utility/Enums.hpp>
#include <Nazara/Utility/VertexStruct.hpp>
#include <Nazara/Utility/IndexMapper.hpp>
#include <Nazara/Utility/Mesh.hpp>
#include <Nazara/Utility/StaticMesh.hpp>
#include <cassert>

template <typename T>
Nz::ModelRef render(const SphereSurface<T> & surface)
{
	auto points = indexCount(surface);

	Nz::IndexBufferRef indexs = Nz::IndexBuffer::New(false, points, Nz::DataStorage_Hardware);
	Nz::VertexBufferRef vertexs = Nz::VertexBuffer::New(Nz::VertexDeclaration::Get(Nz::VertexLayout_XYZ_Normal_UV_Tangent), points, Nz::DataStorage_Hardware);

	Nz::BufferMapper<Nz::VertexBuffer> vertexsMapper(vertexs, Nz::BufferAccess_WriteOnly);
	Nz::VertexStruct_XYZ_Normal_UV_Tangent* meshVertices = static_cast<Nz::VertexStruct_XYZ_Normal_UV_Tangent*>(vertexsMapper.GetPointer());
	Nz::IndexMapper indexsMapper(indexs, Nz::BufferAccess_WriteOnly);

	unsigned int triangleIndex = 0;
	for (auto it(surface.trianglesBegin()); it != surface.trianglesEnd(); it++)
	{
		SphereBlock<T> b1(*std::next(surface.blocksBegin(), it->block1));
		SphereBlock<T> b2(*std::next(surface.blocksBegin(), it->block2));
		SphereBlock<T> b3(*std::next(surface.blocksBegin(), it->block3));

		Nz::Vector3f pos1(toVector3(b1.pos, surface.radius()) + offset(b1, surface.radius()));
		Nz::Vector3f pos2(toVector3(b2.pos, surface.radius()) + offset(b2, surface.radius()));
		Nz::Vector3f pos3(toVector3(b3.pos, surface.radius()) + offset(b3, surface.radius()));

		Nz::Vector3f normal = Nz::Vector3f::CrossProduct(pos2 - pos1, pos3 - pos1);

		meshVertices[triangleIndex * 3].position = pos1;
		meshVertices[triangleIndex * 3 + 1].position = pos2;
		meshVertices[triangleIndex * 3 + 2].position = pos3;
		meshVertices[triangleIndex * 3].normal = normal;
		meshVertices[triangleIndex * 3 + 1].normal = normal;
		meshVertices[triangleIndex * 3 + 2].normal = normal;

		indexsMapper.Set(triangleIndex * 3, triangleIndex * 3);
		indexsMapper.Set(triangleIndex * 3 + 1, triangleIndex * 3 + 1);
		indexsMapper.Set(triangleIndex * 3 + 2, triangleIndex * 3 + 2);

		triangleIndex++;
	}

	vertexsMapper.Unmap();
	indexsMapper.Unmap();

	Nz::MeshRef mesh = Nz::Mesh::New();
	mesh->CreateStatic();

	Nz::StaticMeshRef subMesh = Nz::StaticMesh::New(mesh);
	if (!subMesh->Create(vertexs))
		assert(false);

	subMesh->GenerateAABB();
	subMesh->SetIndexBuffer(indexs);
	subMesh->SetPrimitiveMode(Nz::PrimitiveMode_TriangleList);
	subMesh->SetMaterialIndex(0);

	mesh->AddSubMesh(subMesh);
	
	//Nz::SubMeshRef t = mesh->BuildSubMesh(Nz::Primitive::IcoSphere(surface.radius()/2));
	//t->SetMaterialIndex(0);

	Nz::ModelRef model = Nz::Model::New();
	model->SetMesh(mesh);

	Nz::MaterialRef mat = Nz::Material::New();
	mat->SetShader("PhongLighting");
	mat->SetDiffuseColor(Nz::Color::Red);
	//mat->SetSpecularColor(Nz::Color::Black);
	mat->SetFaceFilling(Nz::FaceFilling_Fill);
	model->SetMaterial(0, mat);

	return model;
}

template <typename T>
unsigned int indexCount(const SphereSurface<T> & surface)
{
	return std::distance(surface.trianglesBegin(), surface.trianglesEnd())*3;
}

template<>
Nz::Vector3f offset(const SphereBlock<float> & point, float radius)
{
	Nz::Vector3f pos(toVector3(point.pos));
	return pos.Normalize()*radius*point.data;
}