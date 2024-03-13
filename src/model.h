#ifndef MODEL_H
#define MODEL_H

#include <vendor/assimp/Importer.hpp>
#include <vendor/assimp/scene.h>
#include <vendor/assimp/postprocess.h>

#include "rtweekend.h"
#include "triangle.h"
#include "hittable_list.h"
#include "material.h"

#include <string>

struct mesh{
    mesh(std::vector<vec3> &vert, std::vector<int> &ind) {
        vertices =vert;
        indices = ind;
    }
    
    std::vector<vec3> vertices;
    std::vector<int> indices;
};
//basic model loading without materials or textures;
class model{
public:
    model(const std::string &path, double scale_ = 1.0, std::shared_ptr<material> mat = std::make_shared<lambertian>(vec3(0)) )
        : scale(scale_), mtr(mat)
    {
        Assimp::Importer import;
        const aiScene *scene = import.ReadFile(path, aiProcess_Triangulate);
        if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
            {
                std::cerr << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
                return;
            }
        directory = path.substr(0, path.find_last_of('/'));

        processNode(scene->mRootNode, scene);
    }
    hittable_list getHittableList(){
        hittable_list triangles;
        for(auto m: meshes){
            for(int i = 0; i<m.indices.size(); i+=3){
                triangles.add(std::make_shared<rotate_>(std::make_shared<triangle>
                    (
                    scale*m.vertices[m.indices[i+0]],
                    scale*m.vertices[m.indices[i+1]],
                    scale*m.vertices[m.indices[i+2]],
                    mtr
                    ),
                    0, 0, 0
                ));
            }
        }
        return triangles;
    }
    hittable_list getHittableList(vec3 rotate, vec3 translation){
        hittable_list triangles;
        for(auto m: meshes){
            for (int i = 0; i < m.indices.size(); i += 3) {
                std::shared_ptr<hittable> currTriangle = std::make_shared<triangle>
                    (
                        scale * m.vertices[m.indices[i + 0]],
                        scale * m.vertices[m.indices[i + 1]],
                        scale * m.vertices[m.indices[i + 2]],
                        mtr
                    );

                currTriangle = std::make_shared<rotate_>(currTriangle, rotate.x(), rotate.y(), rotate.z());
                currTriangle = std::make_shared<translate>(currTriangle, translation);

                triangles.add(currTriangle);
            }
        }
        return triangles;
    }
private:
    std::string directory;
    std::vector<mesh> meshes;
    std::shared_ptr<material> mtr;
    double scale;
    
    void processNode(aiNode *node, const aiScene *scene){
        for(int i=0;i<node->mNumMeshes;i++){
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            processMesh(mesh, scene);
        }
        for(int i=0;i<node->mNumChildren;i++){
            processNode(node->mChildren[i], scene);
        }
    }
    void processMesh(aiMesh *aimesh, const aiScene *scene){
        
        std::vector<vec3> vertices;
        std::vector<int> indices;
        
        for(int i=0;i<aimesh->mNumVertices;i++){
            
            vec3 pos((double)aimesh->mVertices[i].x,
                     (double)aimesh->mVertices[i].y,
                     -(double)aimesh->mVertices[i].z);
            
            vertices.push_back(pos);
        }
        for(int i = 0; i < aimesh->mNumFaces; i++)
        {
            aiFace face = aimesh->mFaces[i];
            for(int j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }
        meshes.push_back(mesh(vertices, indices));
    }
};

#endif // MODEL_H