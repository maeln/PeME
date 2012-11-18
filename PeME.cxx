// PeME.cxx
// 
// Copyright 2012 Mael N. <contact@maeln.com>
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
// MA 02110-1301, USA.

// TO DO:
	// - Texture coordinate.
	// - adapting the default name if more than 1 mesh had no name.
	// - Add the support of compression via zlib.

#include <iostream>
#include <fstream>
#include <string>
#include <stdexcept>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

void write_mesh(aiMesh* mesh, std::string filename)
{
	if(!mesh->HasFaces() || !mesh->HasNormals())
		std::runtime_error("Mesh is missing faces or normals.");
	
	unsigned int numVert = mesh->mNumVertices;
	// unsigned int numNormals = mesh.mNormals.Length(); // mNormals is the size of mNumVertices, see the documentation.
	aiString name = mesh->mName;
	if(name.length == 0) // On évite que le nom du mesh soit vide et rendent un fichier invalide.
	{
		std::cerr << "Your mesh has no name and will be named 'mesh' by default." << std::endl;
		name = "mesh";
	}
	
	aiVector3D* normals = mesh->mNormals;
	aiVector3D* vertices = mesh->mVertices;
	
	std::ofstream file(filename.c_str(), std::ios::out | std::ios::ate); // On ajoute les arrays en fin de fichier, évite de supprimer tout le fichier si plusieurs mesh dans le même fichier.
	if(file)
	{
		file << "// Vertices of the meshs" << std::endl;
		file << "float " << name.C_Str() << "[] = {" << std::endl; // On créer un tableaux du type : float array[] = { x, y, z, ... };
		for(unsigned int n = 0; n <= (numVert-1); n++)
		{
			file << "\t" << vertices[n].x << "," << vertices[n].y << "," << vertices[n].z << "," << std::endl;
		}
		file << "};" << std::endl;
		
		file << "// Normals of the meshs" << std::endl; // Pareille pour les normales.
		file << "float " << name.C_Str() << "_normals" << "[] = {" << std::endl;
		for(unsigned int n = 0; n <= (numVert-1); n++)
		{
			file << "\t" << normals[n].x << "," << normals[n].y << "," << normals[n].z << "," << std::endl;
		}
		file << "};" << std::endl;
		
		if(mesh->HasTextureCoords(0)) // Si le fichier à des coordonnées pour les texture on les écrits aussi 
		{
			unsigned int* nbUV = mesh->mNumUVComponents;
			aiVector3D** uvCoord = mesh->mTextureCoords;
			for(unsigned int i = 0; i < 0x8; i++)
			{
				if(nbUV[i] != 0)
				{
					file << "// Textcoords for UV channel " << i << std::endl; // Pareille pour les normales.
					file << "unsigned int " << name.C_Str() << "_textDimension" << i << "(" << nbUV[i] << ");" << std::endl;
					file << "float " << name.C_Str() << "_texcords" << i << "[] = {" << std::endl;
					for(unsigned int n = 0; n <= (numVert-1); n++)
					{
						file << "\t";
						file << uvCoord[i][n].x << ",";
						if(nbUV[0] >= 2)
							file << uvCoord[i][n].y << ",";
						if(nbUV[0] >= 3)
							file << uvCoord[i][n].z << ",";
						
						file << std::endl;
					}
					file << "};" << std::endl;
				}
			}
		}
		file.close();
	}
	else
		std::runtime_error("Can't open / create the output file.");
}

int main(int argc, char **argv)
{
	// commande :
	// PeME [-s] <fichier 3D> <fichier C de sortie>
	
	if(argc > 1)
	{
		Assimp::Importer importer;
		const aiScene* scene;
		bool third; // On veut vérifier si le fichier de sortie est le 3eme argument ( c'est le cas si on utilise -s ).
		
		if(argv[1] == "-s")
		{
			// On lit le fichier, on décompose toute les faces en triangles, on joint les vertices identique ( normalement plus besoin d'index suite à ça ) et on génère les normales.
			scene = importer.ReadFile(argv[2], aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_GenSmoothNormals | aiProcess_SortByPType );
			third = true;
		}
		else
		{
			scene = importer.ReadFile(argv[1], aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_GenNormals | aiProcess_SortByPType );
			third = false;
		}
		
		// Un array de scene ? 
		if(scene->HasMeshes())
		{
			unsigned int numMeshes = scene->mNumMeshes;
			aiMesh** dispMesh = scene->mMeshes;
			for(unsigned int n = 0; n < numMeshes; n++) // Pour chaque mesh de la scène on va écrire un array de vertices et de normales.
			{
				if(third)
					write_mesh(dispMesh[n], argv[3]);
				else
					write_mesh(dispMesh[n], argv[2]);
			}
		}
		else
		{
			std::runtime_error("No meshes.");
			return 1;
		}
	}
	else
	{
		std::runtime_error("No argument provided.");
		return 1;
	}
	
	return 0;
}

