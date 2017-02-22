#include "Render.h"

Nz::Vector3f offset(const SphereBlock<float> & point, float radius)
{
	Nz::Vector3f pos(toVector3(point.pos));
	return pos.Normalize()*radius*point.data;
}

Nz::Vector3f offset(const SphereBlock<BlockInfo> & point, float radius)
{
	Nz::Vector3f pos(toVector3(point.pos));
	return pos.Normalize()*radius*point.data.height;
}

void createWater(const Planet & p, Nz::MeshRef & mesh)
{
	std::vector<unsigned int> triangles;
	for (unsigned int i(0); i < p.triangleCount(); i++)
	{
		auto & triangle(p.triangle(i));
		if (p.biome(p.block(triangle.block1).data.biomeIndex).type() == BiomeType::OCEAN ||
			p.biome(p.block(triangle.block2).data.biomeIndex).type() == BiomeType::OCEAN ||
			p.biome(p.block(triangle.block3).data.biomeIndex).type() == BiomeType::OCEAN)
			triangles.push_back(i);
	}

	if (triangles.empty())
		return;
	Nz::Vector2f uv(colorToUV(p.biome(p.block(p.triangle(triangles.front()).block1).data.biomeIndex).getColor()));

	Nz::IndexBufferRef indexs = Nz::IndexBuffer::New(true, triangles.size()*3, Nz::DataStorage_Hardware, 0);
	Nz::VertexBufferRef vertexs = Nz::VertexBuffer::New(Nz::VertexDeclaration::Get(Nz::VertexLayout_XYZ_Normal_UV_Tangent), triangles.size()*3, Nz::DataStorage_Hardware, 0);

	Nz::BufferMapper<Nz::VertexBuffer> vertexsMapper(vertexs, Nz::BufferAccess_WriteOnly);
	Nz::VertexStruct_XYZ_Normal_UV_Tangent* meshVertices = static_cast<Nz::VertexStruct_XYZ_Normal_UV_Tangent*>(vertexsMapper.GetPointer());
	Nz::IndexMapper indexsMapper(indexs, Nz::BufferAccess_WriteOnly);

	for (unsigned int i(0); i < triangles.size(); i++)
	{
		auto & triangle(p.triangle(triangles[i]));
		auto & b1(p.block(triangle.block1));
		auto & b2(p.block(triangle.block2));
		auto & b3(p.block(triangle.block3));

		Nz::Vector3f pos1(toVector3(b1.pos, p.radius()));
		Nz::Vector3f pos2(toVector3(b2.pos, p.radius()));
		Nz::Vector3f pos3(toVector3(b3.pos, p.radius()));

		meshVertices[i * 3].position = pos1;
		meshVertices[i * 3 + 1].position = pos2;
		meshVertices[i * 3 + 2].position = pos3;
		Nz::Vector3f normal = Nz::Vector3f::CrossProduct(pos2 - pos1, pos3 - pos1);
		meshVertices[i * 3].normal = normal;
		meshVertices[i * 3 + 1].normal = normal;
		meshVertices[i * 3 + 2].normal = normal;
		meshVertices[i * 3].uv = uv;
		meshVertices[i * 3 + 1].uv = uv;
		meshVertices[i * 3 + 2].uv = uv;

		indexsMapper.Set(i * 3, i * 3);
		indexsMapper.Set(i * 3 + 1, i * 3 + 1);
		indexsMapper.Set(i * 3 + 2, i * 3 + 2);
	}

	vertexsMapper.Unmap();
	indexsMapper.Unmap();

	Nz::StaticMeshRef subMesh = Nz::StaticMesh::New(mesh);
	if (!subMesh->Create(vertexs))
		assert(false);

	subMesh->GenerateAABB();
	subMesh->SetIndexBuffer(indexs);
	subMesh->SetPrimitiveMode(Nz::PrimitiveMode_TriangleList);
	subMesh->SetMaterialIndex(1);

	mesh->AddSubMesh(subMesh);
}

Nz::ModelRef render(const Planet & p)
{
	auto points = indexCount(p);

	Nz::IndexBufferRef indexs = Nz::IndexBuffer::New(true, points, Nz::DataStorage_Hardware, 0);
	Nz::VertexBufferRef vertexs = Nz::VertexBuffer::New(Nz::VertexDeclaration::Get(Nz::VertexLayout_XYZ_Normal_UV_Tangent), points, Nz::DataStorage_Hardware, 0);

	Nz::BufferMapper<Nz::VertexBuffer> vertexsMapper(vertexs, Nz::BufferAccess_WriteOnly);
	Nz::VertexStruct_XYZ_Normal_UV_Tangent* meshVertices = static_cast<Nz::VertexStruct_XYZ_Normal_UV_Tangent*>(vertexsMapper.GetPointer());
	Nz::IndexMapper indexsMapper(indexs, Nz::BufferAccess_WriteOnly);

	unsigned int triangleIndex = 0;
	for (auto it(p.trianglesBegin()); it != p.trianglesEnd(); it++)
	{
		SphereBlock<BlockInfo> b1(*std::next(p.blocksBegin(), it->block1));
		SphereBlock<BlockInfo> b2(*std::next(p.blocksBegin(), it->block2));
		SphereBlock<BlockInfo> b3(*std::next(p.blocksBegin(), it->block3));

		Nz::Vector3f pos1(toVector3(b1.pos, p.radius()) + offset(b1, p.radius()));
		Nz::Vector3f pos2(toVector3(b2.pos, p.radius()) + offset(b2, p.radius()));
		Nz::Vector3f pos3(toVector3(b3.pos, p.radius()) + offset(b3, p.radius()));

		Nz::Vector3f normal = Nz::Vector3f::CrossProduct(pos2 - pos1, pos3 - pos1);

		meshVertices[triangleIndex * 3].position = pos1;
		meshVertices[triangleIndex * 3 + 1].position = pos2;
		meshVertices[triangleIndex * 3 + 2].position = pos3;
		meshVertices[triangleIndex * 3].normal = normal;
		meshVertices[triangleIndex * 3 + 1].normal = normal;
		meshVertices[triangleIndex * 3 + 2].normal = normal;
		Nz::Vector2f uv(colorToUV(p.biome(b1.data.biomeIndex).getColor()));
		meshVertices[triangleIndex * 3].uv = uv;
		meshVertices[triangleIndex * 3 + 1].uv = uv;
		meshVertices[triangleIndex * 3 + 2].uv = uv;

		indexsMapper.Set(triangleIndex * 3, triangleIndex * 3);
		indexsMapper.Set(triangleIndex * 3 + 1, triangleIndex * 3 + 1);
		indexsMapper.Set(triangleIndex * 3 + 2, triangleIndex * 3 + 2);

		triangleIndex++;
	}

	vertexsMapper.Unmap();
	indexsMapper.Unmap();

	Nz::MeshRef mesh = Nz::Mesh::New();
	mesh->CreateStatic();
	mesh->SetMaterialCount(2);

	Nz::StaticMeshRef subMesh = Nz::StaticMesh::New(mesh);
	if (!subMesh->Create(vertexs))
		assert(false);

	subMesh->GenerateAABB();
	subMesh->SetIndexBuffer(indexs);
	subMesh->SetPrimitiveMode(Nz::PrimitiveMode_TriangleList);
	subMesh->SetMaterialIndex(0);

	mesh->AddSubMesh(subMesh);

	createWater(p, mesh);

	//Nz::SubMeshRef t = mesh->BuildSubMesh(Nz::Primitive::IcoSphere(surface.radius()/2));
	//t->SetMaterialIndex(0);

	Nz::ModelRef model = Nz::Model::New();

	model->SetMesh(mesh);

	Nz::MaterialRef mat = Nz::Material::New();
	mat->LoadFromFile("Res/512.png");
	Nz::TextureSampler tex(mat->GetDiffuseSampler());
	tex.SetFilterMode(Nz::SamplerFilter_Nearest);
	mat->SetDiffuseSampler(tex);
	mat->SetFaceFilling(Nz::FaceFilling_Fill);
	mat->SetShader("PhongLighting");
	model->SetMaterial(0, mat);
	
	//alpha for water
	Nz::MaterialRef matWater = Nz::Material::New("Translucent3D");
	matWater->SetDiffuseMap("Res/512.png");
	matWater->SetDiffuseColor(Nz::Color(255, 255, 255, 150));
	Nz::TextureSampler texWater(matWater->GetDiffuseSampler());
	texWater.SetFilterMode(Nz::SamplerFilter_Nearest);
	matWater->SetDiffuseSampler(texWater);
	matWater->EnableFaceCulling(true);
	matWater->SetFaceFilling(Nz::FaceFilling_Fill);
	matWater->SetShader("PhongLighting");
	model->SetMaterial(1, matWater);

	return model;
}