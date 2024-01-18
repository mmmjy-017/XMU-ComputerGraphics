//////////////////////////////////////////////////////////////////////////////////
// OBJmodelViewer.cpp
//
// An object defined in an external Wavefront OBJ file is loaded and displayed. 
// Only vertex and face lines are read. All other lines are ignored. Vertex lines 
// are assumed to have only x, y and z coordinate values. The (optional) w value 
// is ignored if present. Within a face line only vertex indices are read. Texture 
// and normal indices are allowed but ignored. Face lines can have more than three 
// vertices. If a face line has more than three vertices the output is a fan 
// triangulation. Therefore, the mesh generated consists of only triangles.
//
// Interaction:
// Press x, X, y, Y, z, Z to turn the object.
//
// Sumanta Guha.
//////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <sstream>    
#include <string> 
#include <fstream> 
#include <vector>

#include <GL/glew.h>
#include <GL/freeglut.h> 
#include <cmath>
#include"Mesh3D.h"

#define M_PI 3.1415926
GLfloat radians_matrix[16];

using namespace std;

// Globals.
static std::vector<float> verticesVector; // Vector to read in vertex x, y and z values fromt the OBJ file.
static std::vector<int> facesVector; // Vector to read in face vertex indices from the OBJ file.
static float *vertices = NULL;  // Vertex array of the object x, y, z values.
static int *faces = NULL; // Face (triangle) vertex indices.
static int numIndices; // Number of face vertex indices.
static float Xangle = 0.0, Yangle = 0.0, Zangle = 0.0; // Angles to rotate the object.
int change = 0;   //切换平滑着色和平面着色

// Routine to read a Wavefront OBJ file. 
// Only vertex and face lines are processed. All other lines,including texture, 
// normal, material, etc., are ignored.
// Vertex lines are assumed to have only x, y, z coordinate values and no w value.
// If a w value is present it is ignored.
// Within a face line only vertex indices are read. Texture and normal indices are 
// allowed but ignored. Face lines can have more than three vertices.
//
// OUTPUT: The vertex coordinate values are written to a vector "verticesVector" 
// (this name is hardcoded in the routine so the calling program should take note). 
// The face vertex indices are written to a vector "facesVector" (hardcoded name).
// Faces with more than three vertices are fan triangulated about the first vertex
// and the triangle indices written. In other words, output faces are all triangles.
// All vertex indices are decremented by 1 to make the index range start from 0.


#include "Mesh3D.h"

#include <fstream>
#include <iostream>
#include <xutility>

#define SWAP(a,b,T) {T tmp=(a); (a)=(b); (b)=tmp;}
#define min(a,b) a<b?a:b
#define max(a,b) a>b?a:b

int w = 600, h = 500;//视角高宽
Mesh3D* ptr_mesh_ = new Mesh3D();

Mesh3D::Mesh3D(void)
{
	// intialization
	pvertices_list_ = NULL;
	pfaces_list_ = NULL;
	pedges_list_ = NULL;

	xmax_ = ymax_ = zmax_ = 1.f;
	xmin_ = ymin_ = zmin_ = -1.f;

	num_components_ = 0;
	average_edge_length_ = 1.f;
}

void Mesh3D::ClearData(void)
{
	ClearVertex();
	ClearEdges();
	ClearFaces();
	edgemap_.clear();

	xmax_ = ymax_ = zmax_ = 1.f;
	xmin_ = ymin_ = zmin_ = -1.f;
}

void Mesh3D::ClearVertex(void)
{

	if (pvertices_list_ == NULL)
	{
		return;
	}
	else
	{
		for (VERTEX_ITER viter = pvertices_list_->begin(); viter != pvertices_list_->end(); viter++)
		{
			if (*viter != NULL)
			{
				delete* viter;
				*viter = NULL;
			}
			else
			{
				// ERROR
			}
		}
		delete pvertices_list_;
		pvertices_list_ = NULL;
	}
}

void Mesh3D::ClearEdges(void)
{
	if (pedges_list_ == NULL)
	{
		return;
	}
	else
	{
		for (EDGE_ITER eiter = pedges_list_->begin(); eiter != pedges_list_->end(); eiter++)
		{
			if (*eiter != NULL)
			{
				delete* eiter;
				*eiter = NULL;
			}
			else
			{
				// ERROR
			}
		}
		delete pedges_list_;
		pedges_list_ = NULL;
	}
}

void Mesh3D::ClearFaces(void)
{
	if (pfaces_list_ == NULL)
	{
		return;
	}
	else
	{
		for (FACE_ITER fiter = pfaces_list_->begin(); fiter != pfaces_list_->end(); fiter++)
		{
			if (*fiter != NULL)
			{
				delete* fiter;
				*fiter = NULL;
			}
			else
			{
				// ERROR
			}
		}
		delete pfaces_list_;
		pfaces_list_ = NULL;
	}
}

HE_vert* Mesh3D::InsertVertex(const Vec3f& v)
{
	HE_vert* pvert = new HE_vert(v);
	if (pvertices_list_ == NULL)
	{
		pvertices_list_ = new std::vector<HE_vert*>;
	}
	pvert->id_ = static_cast<int>(pvertices_list_->size());
	pvertices_list_->push_back(pvert);
	return pvert;
}

HE_edge* Mesh3D::InsertEdge(HE_vert* vstart, HE_vert* vend)
{
	if (vstart == NULL || vend == NULL)
	{
		return NULL;
	}

	if (pedges_list_ == NULL)
	{
		pedges_list_ = new std::vector<HE_edge*>;
	}

	if (edgemap_[PAIR_VERTEX(vstart, vend)] != NULL)
	{
		return edgemap_[PAIR_VERTEX(vstart, vend)];
	}

	HE_edge* pedge = new HE_edge;
	pedge->pvert_ = vend;
	pedge->pvert_->degree_++;
	vstart->pedge_ = pedge;
	edgemap_[PAIR_VERTEX(vstart, vend)] = pedge;

	pedge->id_ = static_cast<int>(pedges_list_->size());
	pedges_list_->push_back(pedge);

	return pedge;
}

HE_face* Mesh3D::InsertFace(std::vector<HE_vert* >& vec_hv)
{
	int vsize = static_cast<int>(vec_hv.size());
	//if (vsize != 3)
	//{
	//	return NULL;
	//}

	if (pfaces_list_ == NULL)
	{
		pfaces_list_ = new std::vector<HE_face*>;
	}

	HE_face* pface = new HE_face;
	pface->valence_ = vsize;
	VERTEX_ITER viter = vec_hv.begin();
	VERTEX_ITER nviter = vec_hv.begin();
	nviter++;

	HE_edge* he1 = NULL, * he2 = NULL;
	std::vector<HE_edge*> vec_edges;
	int i = 0;
	for (i = 0; i < vsize - 1; i++)
	{
		he1 = InsertEdge(*viter, *nviter);
		he2 = InsertEdge(*nviter, *viter);

		if (pface->pedge_ == NULL)
			pface->pedge_ = he1;

		he1->pface_ = pface;
		he1->ppair_ = he2;
		he2->ppair_ = he1;
		vec_edges.push_back(he1);
		viter++, nviter++;
	}

	nviter = vec_hv.begin();

	he1 = InsertEdge(*viter, *nviter);
	he2 = InsertEdge(*nviter, *viter);
	he1->pface_ = pface;
	if (pface->pedge_ == NULL)
		pface->pedge_ = he1;

	he1->ppair_ = he2;
	he2->ppair_ = he1;
	vec_edges.push_back(he1);

	for (i = 0; i < vsize - 1; i++)
	{
		vec_edges[i]->pnext_ = vec_edges[i + 1];
		vec_edges[i + 1]->pprev_ = vec_edges[i];
	}
	vec_edges[i]->pnext_ = vec_edges[0];
	vec_edges[0]->pprev_ = vec_edges[i];

	pface->id_ = static_cast<int>(pfaces_list_->size());
	pfaces_list_->push_back(pface);

	return pface;
}

bool Mesh3D::LoadFromOBJFile(const char* fins)
{
	//	cout << "Loading......." << endl;
	FILE* pfile = fopen(fins, "r");

	char* tok;
	//char *tok_tok;
	char temp[128];

	try
	{
		ClearData();
		//read vertex
		fseek(pfile, 0, SEEK_SET);
		char pLine[512];

		while (fgets(pLine, 512, pfile))
		{
			if (pLine[0] == 'v' && pLine[1] == ' ')
			{
				Vec3f nvv;
				tok = strtok(pLine, " ");
				for (int i = 0; i < 3; i++)
				{
					tok = strtok(NULL, " ");
					strcpy(temp, tok);
					temp[strcspn(temp, " ")] = 0;
					nvv[i] = (float)atof(temp);
				}
				InsertVertex(nvv);
			}
		}

		//read facets
		fseek(pfile, 0, SEEK_SET);

		while (fgets(pLine, 512, pfile))
		{
			char* pTmp = pLine;
			if (pTmp[0] == 'f')
			{
				std::vector<HE_vert* > s_faceid;

				tok = strtok(pLine, " ");
				while ((tok = strtok(NULL, " ")) != NULL)
				{
					strcpy(temp, tok);
					temp[strcspn(temp, "/")] = 0;
					int id = (int)strtol(temp, NULL, 10) - 1;
					HE_vert* hv = get_vertex(id);
					bool findit = false;
					for (int i = 0; i < (int)s_faceid.size(); i++)
					{
						if (hv == s_faceid[i])	//remove redundant vertex id if it exists
						{
							//	cout << "remove redundant vertex" << endl;
							findit = true;
							break;
						}
					}
					if (findit == false && hv != NULL)
					{
						s_faceid.push_back(hv);
					}
				}
				if ((int)s_faceid.size() >= 3)
				{
					InsertFace(s_faceid);
				}
			}
		}

		//read texture coords
		fseek(pfile, 0, SEEK_SET);
		std::vector<Vec3f> texCoordsTemp;
		while (fscanf(pfile, "%s", pLine) != EOF)
		{
			if (pLine[0] == 'v' && pLine[1] == 't')
			{
				Vec3f texTemp(0.f, 0.f, 0.f);
				fscanf(pfile, "%f %f", &texTemp[0], &texTemp[1]);
				texCoordsTemp.push_back(texTemp);
			}
		}
		//read texture index

		if (texCoordsTemp.size() > 0)
		{
			fseek(pfile, 0, SEEK_SET);

			int faceIndex = 0;
			while (fscanf(pfile, "%s", pLine) != EOF)
			{

				if (pLine[0] == 'f')
				{
					int v, t;
					fscanf(pfile, "%s", pLine);
					if (sscanf(pLine, "%d/%d", &v, &t) == 2)
					{
						std::map<int, int> v2tex;
						v2tex[v - 1] = t - 1;

						fscanf(pfile, "%s", pLine);
						sscanf(pLine, "%d/%d", &v, &t);
						v2tex[v - 1] = t - 1;

						fscanf(pfile, "%s", pLine);
						sscanf(pLine, "%d/%d", &v, &t);
						v2tex[v - 1] = t - 1;

						HE_edge* edgeTemp = pfaces_list_->at(faceIndex)->pedge_;
						edgeTemp->texCoord_ = texCoordsTemp.at(v2tex[edgeTemp->pvert_->id_]);
						edgeTemp->pvert_->texCoord_ = edgeTemp->texCoord_;
						edgeTemp = edgeTemp->pnext_;
						edgeTemp->texCoord_ = texCoordsTemp.at(v2tex[edgeTemp->pvert_->id_]);
						edgeTemp->pvert_->texCoord_ = edgeTemp->texCoord_;
						edgeTemp = edgeTemp->pnext_;
						edgeTemp->texCoord_ = texCoordsTemp.at(v2tex[edgeTemp->pvert_->id_]);
						edgeTemp->pvert_->texCoord_ = edgeTemp->texCoord_;
						faceIndex++;
					}
				}
			}
		}

		//cout << vertex_list->size() << " vertex, " << faces_list->size() << " faces " << endl;

		UpdateMesh();
		Unify(2.f);
	}
	catch (...)
	{
		ClearData();
		xmax_ = ymax_ = zmax_ = 1.f;
		xmin_ = ymin_ = zmin_ = -1.f;

		fclose(pfile);
		return false;
	}

	fclose(pfile);

	return isValid();
}

void Mesh3D::WriteToOBJFile(const char* fouts)
{
	std::ofstream fout(fouts);

	fout << "g object\n";
	fout.precision(16);
	//output coordinates of each vertex
	VERTEX_ITER viter = pvertices_list_->begin();
	for (; viter != pvertices_list_->end(); viter++)
	{
		fout << "v " << std::scientific << (*viter)->position_.x()
			<< " " << (*viter)->position_.y() << " " << (*viter)->position_.z() << "\n";
	}

	// 		for (viter = pvertices_list_->begin();viter!=pvertices_list_->end(); viter++) 
	// 		{
	// 			fout<<"vn "<< std::scientific <<(*viter)->normal_.x() 
	// 				<<" "<<(*viter)->normal_.y() <<" "<<(*viter)->normal_.z() <<"\n";
	// 		}
	//output the valence of each face and its vertices_list' id

	FACE_ITER fiter = pfaces_list_->begin();

	for (; fiter != pfaces_list_->end(); fiter++)
	{
		fout << "f";

		HE_edge* edge = (*fiter)->pedge_;

		do {
			fout << " " << edge->ppair_->pvert_->id_ + 1;
			edge = edge->pnext_;

		} while (edge != (*fiter)->pedge_);
		fout << "\n";
	}

	fout.close();
}

void Mesh3D::UpdateMesh(void)
{
	if (!isValid())
	{
		std::cout << "Invalid" << "\n";
		return;
	}
	SetBoundaryFlag();
	BoundaryCheck();
	UpdateNormal();
	ComputeBoundingBox();
	ComputeAvarageEdgeLength();
	SetNeighbors();
}

void Mesh3D::SetBoundaryFlag(void)
{
	for (EDGE_ITER eiter = pedges_list_->begin(); eiter != pedges_list_->end(); eiter++)
	{
		if ((*eiter)->pface_ == NULL)
		{
			(*eiter)->set_boundary_flag(BOUNDARY);
			(*eiter)->ppair_->set_boundary_flag(BOUNDARY);
			(*eiter)->pvert_->set_boundary_flag(BOUNDARY);
			(*eiter)->ppair_->pvert_->set_boundary_flag(BOUNDARY);
			(*eiter)->ppair_->pface_->set_boundary_flag(BOUNDARY);
		}
	}
}

void Mesh3D::BoundaryCheck()
{
	for (VERTEX_ITER viter = pvertices_list_->begin(); viter != pvertices_list_->end(); viter++)
	{
		if ((*viter)->isOnBoundary())
		{
			HE_edge* edge = (*viter)->pedge_;
			int deg = 0;
			while (edge->pface_ != NULL && deg < (*viter)->degree())
			{
				edge = edge->pprev_->ppair_;
				deg++;
			}
			(*viter)->pedge_ = edge;
		}
	}
}

void Mesh3D::UpdateNormal(void)
{
	ComputeFaceslistNormal();
	ComputeVertexlistNormal();
}

void Mesh3D::ComputeFaceslistNormal(void)
{
	for (FACE_ITER fiter = pfaces_list_->begin(); fiter != pfaces_list_->end(); fiter++)
	{
		ComputePerFaceNormal(*fiter);
	}
}

void Mesh3D::ComputePerFaceNormal(HE_face* hf) {
	//图形学课程上机作业
	//请在此处添加计算面法向量代码
	point a, b, c;
	a = hf->pedge_->pvert_->position();  //得到三角形的三个点
	b = hf->pedge_->pnext_->pvert_->position();
	c = hf->pedge_->pnext_->pnext_->pvert_->position();
	point m1, m2;  //得到两条边向量
	m1 = b - a;
	m2 = c - a;

	double x = m1[1] * m2[2] - m1[2] * m2[1]; //叉乘得到法向量
	double y = m1[2] * m2[0] - m1[0] * m2[2];
	double z = m1[0] * m2[1] - m1[1] * m2[0];

	double norm = sqrt(x * x + y * y + z * z);

	hf->facevector[0] = x / norm;
	hf->facevector[1] = y / norm;
	hf->facevector[2] = z / norm;
}

void Mesh3D::ComputeVertexlistNormal(void)
{
	for (VERTEX_ITER viter = pvertices_list_->begin(); viter != pvertices_list_->end(); viter++)
	{
		ComputePerVertexNormal(*viter);
	}
}

void Mesh3D::ComputePerVertexNormal(HE_vert* hv)
{
	//图形学课程上机作业
	//请在此处添加计算面法向代码
	HE_edge* firstedge = hv->pedge_;
	HE_face* face_ = firstedge->pface_;
	Mesh3D::ComputePerFaceNormal(face_);  //调用函数计算面法向量

	hv->pointvector[0] = 0;  //点向量初始化为0
	hv->pointvector[1] = 0;
	hv->pointvector[2] = 0;

	float total = 0;  //与该顶点相邻的面的数量
	for (int i = 0; i < 3; ++i) {
		hv->pointvector[i] += face_->facevector[i];  //顶点相邻的面法向量之和
	}

	++total;
	HE_edge* nextedge = firstedge->ppair_->pnext_;  //下一条边
	while (nextedge != firstedge) {
		face_ = nextedge->pface_;
		Mesh3D::ComputePerFaceNormal(face_);
		for (int i = 0; i < 3; ++i) {
			hv->pointvector[i] += face_->facevector[i];
		}
		++total;
		nextedge = nextedge->ppair_->pnext_;
	}
	for (int i = 0; i < 3; ++i) {
		hv->pointvector[i] = hv->pointvector[i] / total;
	}
}

void Mesh3D::ComputeBoundingBox(void)
{
	if (pvertices_list_->size() < 3)
	{
		return;
	}

#define MAX_FLOAT_VALUE (static_cast<float>(10e10))
#define MIN_FLOAT_VALUE	(static_cast<float>(-10e10))

	xmax_ = ymax_ = zmax_ = MIN_FLOAT_VALUE;
	xmin_ = ymin_ = zmin_ = MAX_FLOAT_VALUE;

	VERTEX_ITER viter = pvertices_list_->begin();
	for (; viter != pvertices_list_->end(); viter++)
	{
		xmin_ = min(xmin_, (*viter)->position_.x());
		ymin_ = min(ymin_, (*viter)->position_.y());
		zmin_ = min(zmin_, (*viter)->position_.z());
		xmax_ = max(xmax_, (*viter)->position_.x());
		ymax_ = max(ymax_, (*viter)->position_.y());
		zmax_ = max(zmax_, (*viter)->position_.z());
	}
}

void Mesh3D::Unify(float size)
{
	float scaleX = xmax_ - xmin_;
	float scaleY = ymax_ - ymin_;
	float scaleZ = zmax_ - zmin_;
	float scaleMax;

	if (scaleX < scaleY)
	{
		scaleMax = scaleY;
	}
	else
	{
		scaleMax = scaleX;
	}
	if (scaleMax < scaleZ)
	{
		scaleMax = scaleZ;
	}
	float scaleV = size / scaleMax;
	Vec3f centerPos((xmin_ + xmax_) / 2.f, (ymin_ + ymax_) / 2.f, (zmin_ + zmax_) / 2.f);
	for (size_t i = 0; i != pvertices_list_->size(); i++)
	{
		pvertices_list_->at(i)->position_ = (pvertices_list_->at(i)->position_ - centerPos) * scaleV;
	}
}

void Mesh3D::ComputeAvarageEdgeLength(void)
{
	if (!isValid())
	{
		average_edge_length_ = 0.f;
		return;
	}
	float aveEdgeLength = 0.f;
	for (int i = 0; i < num_of_half_edges_list(); i++)
	{
		HE_edge* edge = get_edges_list()->at(i);
		HE_vert* v0 = edge->pvert_;
		HE_vert* v1 = edge->ppair_->pvert_;
		aveEdgeLength += (v0->position() - v1->position()).length();
	}
	average_edge_length_ = aveEdgeLength / num_of_half_edges_list();
	//std::cout << "Average_edge_length = " << average_edge_length_ << "\n";
}

HE_face* Mesh3D::get_face(int vId0, int vId1, int vId2)
{
	HE_vert* v0 = get_vertex(vId0);
	HE_vert* v1 = get_vertex(vId1);
	HE_vert* v2 = get_vertex(vId2);
	if (!v0 || !v1 || !v2)
	{
		return NULL;
	}

	HE_face* face = NULL;

	// 由于对边界点的邻域遍历有bug，所以找到非边界点进行邻域遍历
	if (v0->isOnBoundary())
	{
		if (!v1->isOnBoundary())
		{
			SWAP(v0, v1, HE_vert*);
		}
		else if (!v2->isOnBoundary())
		{
			SWAP(v0, v2, HE_vert*);
		}
		else
		{
			// v0, v1, v2 都是边界点
			// 暂时先不处理
			return NULL;
		}
	}

	if (!v0->isOnBoundary())	// 对边界点的遍历有bug
	{
		HE_edge* edge = v0->pedge_;
		bool inFace = true;
		do
		{
			bool b1 = isFaceContainVertex(edge->pface_, v1);
			bool b2 = isFaceContainVertex(edge->pface_, v2);
			if (!b1 && !b1)
			{
				edge = edge->ppair_->pnext_;
			}
			else if (b1 && b2)
			{
				face = edge->pface_;
				break;
			}
			else
			{
				inFace = false;
				break;
			}
		} while (edge != v0->pedge_ && edge != NULL);
	}

	return face;
}

HE_face* Mesh3D::get_face(const std::vector<unsigned int>& ids)
{
	if (ids.size() < 3)
	{
		std::cout << "can not return face" << std::endl;
		return NULL;
	}
	// 首先找到一个非边界点
	HE_vert* v = NULL;
	for (unsigned int i = 0; i < ids.size(); i++)
	{
		if (!get_vertex(ids[i])->isOnBoundary())
		{
			v = get_vertex(ids[i]);
			break;
		}
	}
	if (!v)
	{
		// 所有点都是边界点
		// 暂不处理
		return NULL;
	}

	HE_edge* edge = v->pedge_;
	HE_face* face = NULL;
	do
	{
		face = edge->pface_;
		edge = edge->ppair_->pnext_;
		bool bInFace = isFaceContainVertex(face, get_vertex(ids[0]));
		if (!bInFace)
		{
			continue;
		}
		for (unsigned int i = 1; i < ids.size(); i++)
		{
			bool b = isFaceContainVertex(face, get_vertex(ids[i]));
			if (b != bInFace)
			{
				bInFace = false;
				break;
			}
		}
		if (bInFace)
		{
			return face;
		}
	} while (edge != v->pedge_ && edge != NULL);
	return NULL;
}

bool Mesh3D::isFaceContainVertex(HE_face* face, HE_vert* vert)
{
	HE_edge* edge = face->pedge_;
	do
	{
		if (edge->pvert_ == vert)
		{
			return true;
		}
		edge = edge->pnext_;
	} while (edge != face->pedge_ && edge != NULL);
	return false;
}

int Mesh3D::GetFaceId(HE_face* face)
{
	return !face ? -1 : face->id();
}

void Mesh3D::ResetFaceSelectedTags(int tag)
{
	for (int i = 0; i < num_of_face_list(); i++)
	{
		get_face(i)->set_selected(tag);
	}
}

void Mesh3D::ResetVertexSelectedTags(int tag)
{
	for (int i = 0; i < num_of_vertex_list(); i++)
	{
		get_vertex(i)->set_seleted(tag);
	}
}

bool Mesh3D::isNeighbors(HE_vert* v0, HE_vert* v1)
{
	if (!v0 || !v1)
	{
		return false;
	}

	HE_edge* edge = v0->pedge_;
	do
	{
		if (edge->pvert_ == v1)
		{
			return true;
		}
		edge = edge->ppair_->pnext_;
	} while (edge != v0->pedge_ && edge);
	return false;
}

int Mesh3D::GetSelectedVrtId()
{
	if (!isValid())
	{
		return -1;
	}
	for (int i = 0; i < num_of_vertex_list(); i++)
	{
		if (get_vertex(i)->selected() == SELECTED)
		{
			return i;
		}
	}
	return -1;
}

void Mesh3D::CreateMesh(const std::vector<Vec3f>& verts, const std::vector<int>& triIdx)
{
	ClearData();
	for (unsigned int i = 0; i < verts.size(); i++)
	{
		InsertVertex(verts[i]);
	}
	for (unsigned int i = 0; i < triIdx.size(); i = i + 3)
	{
		std::vector<HE_vert*> tri;
		HE_vert* v0 = get_vertex(triIdx[i]);
		HE_vert* v1 = get_vertex(triIdx[i + 1]);
		HE_vert* v2 = get_vertex(triIdx[i + 2]);
		if (!v0 || !v1 || !v2) continue;
		tri.push_back(v0);
		tri.push_back(v1);
		tri.push_back(v2);
		InsertFace(tri);
	}
	UpdateMesh();
}

void Mesh3D::CreateMesh(const std::vector<double>& verts, const std::vector<unsigned>& triIdx)
{
	ClearData();
	for (unsigned int i = 0; i < verts.size(); i = i + 3)
	{
		InsertVertex(Vec3f(verts[i], verts[i + 1], verts[i + 2]));
	}
	for (unsigned int i = 0; i < triIdx.size(); i = i + 3)
	{
		std::vector<HE_vert*> tri;
		HE_vert* v0 = get_vertex(triIdx[i]);
		HE_vert* v1 = get_vertex(triIdx[i + 1]);
		HE_vert* v2 = get_vertex(triIdx[i + 2]);
		if (!v0 || !v1 || !v2) continue;
		tri.push_back(v0);
		tri.push_back(v1);
		tri.push_back(v2);
		InsertFace(tri);
	}
	UpdateMesh();
}

int Mesh3D::GetBoundaryVrtSize()
{
	int count = 0;
	for (int i = 0; i < num_of_vertex_list(); i++)
	{
		if (get_vertex(i)->isOnBoundary())
		{
			count++;
		}
	}
	return count;
}

Mesh3D::~Mesh3D(void)
{
	ClearData();
}



void loadOBJ(std::string fileName)
{
   std::string line;
   int count, vertexIndex1, vertexIndex2, vertexIndex3;
   float coordinateValue;
   char currentCharacter, previousCharacter;

   // Open the OBJ file.
   std::ifstream inFile(fileName.c_str(), std::ifstream::in);

   // Read successive lines.
   while (getline(inFile, line))
   {
	  // Line has vertex data.
	  if (line.substr(0, 2) == "v ")
	  {
		 // Initialize a string from the character after "v " to the end.
		 std::istringstream currentString(line.substr(2));

		 // Read x, y and z values. The (optional) w value is not read. 
		 for (count = 1; count <= 3; count++)
		 {
			currentString >> coordinateValue;
			verticesVector.push_back(coordinateValue);
		 }
	  }

	  // Line has face data.
	  else if (line.substr(0, 2) == "f ")
	  {
		 // Initialize a string from the character after "f " to the end.
		 std::istringstream currentString(line.substr(2));

		 // Strategy in the following to detect a vertex index within a face line is based on the
		 // fact that vertex indices are exactly those that follow a white space. Texture and
		 // normal indices are ignored.
		 // Moreover, from the third vertex of a face on output one triangle per vertex, that
		 // being the next triangle in a fan triangulation of the face about the first vertex.
		 previousCharacter = ' ';
		 count = 0;
		 while (currentString.get(currentCharacter))
		 {
			// Stop processing line at comment.
			if ((previousCharacter == '#') || (currentCharacter == '#')) break;

			// Current character is the start of a vertex index.
			if ((previousCharacter == ' ') && (currentCharacter != ' '))
			{
			   // Move the string cursor back to just before the vertex index.
			   currentString.unget();

			   // Read the first vertex index, decrement it so that the index range is from 0, increment vertex counter.
			   if (count == 0)
			   {
				  currentString >> vertexIndex1;
				  vertexIndex1--;
				  count++;
			   }

			   // Read the second vertex index, decrement it, increment vertex counter.
			   else if (count == 1)
			   {
				  currentString >> vertexIndex2;
				  vertexIndex2--;
				  count++;
			   }

			   // Read the third vertex index, decrement it, increment vertex counter AND output the first triangle.
			   else if (count == 2)
			   {
				  currentString >> vertexIndex3;
				  vertexIndex3--;
				  count++;
				  facesVector.push_back(vertexIndex1);
				  facesVector.push_back(vertexIndex2);
				  facesVector.push_back(vertexIndex3);
			   }

			   // From the fourth vertex and on output the next triangle of the fan.
			   else
			   {
				  vertexIndex2 = vertexIndex3;
				  currentString >> vertexIndex3;
				  vertexIndex3--;
				  facesVector.push_back(vertexIndex1);
				  facesVector.push_back(vertexIndex2);
				  facesVector.push_back(vertexIndex3);
			   }

			   // Begin the process of detecting the next vertex index just after the vertex index just read.
			   currentString.get(previousCharacter);
			}

			// Current character is not the start of a vertex index. Move ahead one character.
			else previousCharacter = currentCharacter;
		 }
	  }

	  // Nothing other than vertex and face data is processed.
	  else
	  {
	  }
   }

   // Close the OBJ file.
   inFile.close();
}

// 计算光线和三角面的法向量的点积
vector<double> dot_products;   //alpha
vector<double> light_vector= { 0,0,1};
vector<vector<double> > face_normal_vector;
GLfloat rotation_matrix[16];

// 计算向量的叉乘 得到单位向量
vector<double> cross_product(vector<double> v1, vector<double> v2) {
	double x = v1[1] * v2[2] - v1[2] * v2[1];
	double y = v1[2] * v2[0] - v1[0] * v2[2];
	double z = v1[0] * v2[1] - v1[1] * v2[0];
	double norm = sqrt(x * x + y * y + z * z);
	return { x / norm, y / norm, z / norm};
}

// 计算三角面的法向量
vector<double> face_normal(vector<double> v1, vector<double> v2, vector<double> v3) {
	vector<double> e1 = { v2[0] - v1[0], v2[1] - v1[1], v2[2] - v1[2] };
	vector<double> e2 = { v3[0] - v1[0], v3[1] - v1[1], v3[2] - v1[2] };
	return cross_product(e1, e2);
}

// 计算光线和法向量的点积
double dot_product(vector<double> v1, vector<double> v2) {
	return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
}


void calculate_dot_products(float vertices[], int faces[], vector<double> light_vector) {

	for (int i = 0; i < facesVector.size(); i += 3) {
		vector<double> v1 = { vertices[faces[i] * 3], vertices[faces[i] * 3 + 1], vertices[faces[i] * 3 + 2] };
		vector<double> v2 = { vertices[faces[i + 1] * 3], vertices[faces[i + 1] * 3 + 1], vertices[faces[i + 1] * 3 + 2] };
		vector<double> v3 = { vertices[faces[i + 2] * 3], vertices[faces[i + 2] * 3 + 1], vertices[faces[i + 2] * 3 + 2] };
		vector<double> normal = face_normal(v1, v2, v3);
		face_normal_vector.push_back(normal);
		double dot = dot_product(normal, light_vector);
		if (dot < 0) dot = 0;
		dot_products.push_back(dot);
	}
}


// Initialization routine.
void setup(void)
{
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnable(GL_DEPTH_TEST);
	glClearColor(1.0, 1.0, 1.0, 0.0);

	// Read the external OBJ file into the internal vertex and face vectors.
	bool is_open = ptr_mesh_->LoadFromOBJFile("gourd.obj");

	//// Size the vertex array and copy into it x, y, z values from the vertex vector.
	//vertices = new float[verticesVector.size()];

	//for (int i = 0; i < verticesVector.size(); i++) vertices[i] = verticesVector[i];

	//// Size the faces array and copy into it face index values from the face vector.
	//faces = new int[facesVector.size()];
	//for (int i = 0; i < facesVector.size(); i++) faces[i] = facesVector[i];
	//numIndices = facesVector.size();
	//calculate_dot_products(vertices, faces, light_vector);
}


// Drawing routine.
double Xdelta=0, Ydelta=0, Zdelta=0;
void drawScene(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	gluLookAt(0.0, 0.0, 4.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glMatrixMode(GL_MODELVIEW);
	glColor3f(0.4f, 0.09f, 0.6f);


	// Rotate scene.
	glRotatef(Zangle, 0.0, 0.0, 1.0);
	glRotatef(Yangle, 0.0, 1.0, 0.0);
	glRotatef(Xangle, 1.0, 0.0, 0.0);

	// Draw the object mesh.
	for (int i = 0; i < ptr_mesh_->num_of_face_list(); i = i++)
	{
		point first, second, third;
		first = ptr_mesh_->get_face(i)->pedge_->pvert_->position();//第一个顶点
		second = ptr_mesh_->get_face(i)->pedge_->pnext_->pvert_->position();//第二个点
		third = ptr_mesh_->get_face(i)->pedge_->pnext_->pnext_->pvert_->position();//第三个点
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glBegin(GL_TRIANGLES);//画三角形
		if (change == 0)
		{
			glNormal3fv(ptr_mesh_->get_face(i)->facevector);//平面光照
			glVertex3f(first[0], first[1], first[2]);
			glVertex3f(second[0], second[1], second[2]);
			glVertex3f(third[0], third[1], third[2]);
		}
		else if (change == 1)//平滑光照处理
		{
			glNormal3fv(ptr_mesh_->get_face(i)->pedge_->pvert_->pointvector);
			glVertex3f(first[0], first[1], first[2]);
			glNormal3fv(ptr_mesh_->get_face(i)->pedge_->pnext_->pvert_->pointvector);
			glVertex3f(second[0], second[1], second[2]);
			glNormal3fv(ptr_mesh_->get_face(i)->pedge_->pnext_->pnext_->pvert_->pointvector);
			glVertex3f(third[0], third[1], third[2]);
		}
		glEnd();
	}
	//固定光源
	GLfloat sun_light_position[] = { 0.0f, 0.0f, 4.0f, 1.0f }; //光源的位置在世界坐标系前方，齐次坐标形式
	GLfloat sun_light_ambient[] = { 0.0f, 0.0f, 0.0f, 1.0f };  //RGBA模式的环境光，为0
	GLfloat sun_light_diffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };  //RGBA模式的漫反射光，全白光
	GLfloat sun_light_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f }; //RGBA模式下的镜面光 ，全白光
	glLightfv(GL_LIGHT0, GL_POSITION, sun_light_position);
	glLightfv(GL_LIGHT0, GL_AMBIENT, sun_light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, sun_light_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, sun_light_specular);

	//开启灯光
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);



	//可移动光源
	GLfloat light_pos[] = { 0.0,2.0,0.0,1.0 };
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(40.0, (GLfloat)w / h, 1.0, 100.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
	glEnable(GL_LIGHT1);

	glutSwapBuffers();

	//glGetFloatv(GL_MODELVIEW_MATRIX, rotation_matrix);
	//// Rotate scene.
	//glPushMatrix();
	//glRotatef(Zangle, 0.0, 0.0, 1.0);
	//glRotatef(Yangle, 0.0, 1.0, 0.0);
	//glRotatef(Xangle, 1.0, 0.0, 0.0);
	//glGetFloatv(GL_MODELVIEW_MATRIX, rotation_matrix);
	//glPopMatrix();

	//dot_products.clear();

	//for (int i = 0; i < face_normal_vector.size(); ++i) {
	//	vector<double> normal = face_normal_vector[i];
	//	vector<double>temp(normal);

	//	temp[0] = rotation_matrix[0] * normal[0] + rotation_matrix[1] * normal[1] + rotation_matrix[2] * normal[2] ;//+ rotation_matrix[3] * 0;
	//	temp[1] = rotation_matrix[4] * normal[0] + rotation_matrix[5] * normal[1] + rotation_matrix[6] * normal[2];//+ rotation_matrix[7] * 0;
	//	temp[2] = rotation_matrix[8] * normal[0] + rotation_matrix[9] * normal[1] + rotation_matrix[10] * normal[2];// + rotation_matrix[11] * 0;
	//	//temp[3] = 0;
	//	normal = temp;

	//	double dot = dot_product(normal, light_vector);
	//	if (dot < 0) dot = 0;
	//	dot_products.push_back(dot);
	//}

	//double cl[3] = { 0.4f, 0.09f, 0.6f }; //茄子色

	//glPushMatrix();

	//glRotatef(Zangle, 0.0, 0.0, 1.0);
	//glRotatef(Yangle, 0.0, 1.0, 0.0);
	//glRotatef(Xangle, 1.0, 0.0, 0.0);
	//
	////calculate_dot_products(vertices, faces, light_vector);

	//for (int i = 0; i < facesVector.size(); i += 3) {
	//	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	//	glBegin(GL_TRIANGLES);
	//	double t = dot_products[i / 3];
	//	glColor3f(cl[0] * t, cl[1] * t, cl[2] * t); //染
	//	glVertex3f(vertices[(faces[i]) * 3], vertices[(faces[i]) * 3 + 1], vertices[(faces[i]) * 3 + 2]);
	//	glVertex3f(vertices[(faces[i + 1]) * 3], vertices[(faces[i + 1]) * 3 + 1], vertices[(faces[i + 1]) * 3 + 2]);
	//	glVertex3f(vertices[(faces[i + 2]) * 3], vertices[(faces[i + 2]) * 3 + 1], vertices[(faces[i + 2]) * 3 + 2]);
	//	glEnd();
	//}
	//glPopMatrix();
	//glutSwapBuffers();
}


// OpenGL window reshape routine.
void resize(int w, int h)
{
   glViewport(0, 0, w, h);
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   gluPerspective(60.0, (float)w / (float)h, 1.0, 50.0);
   glMatrixMode(GL_MODELVIEW);
}

void keyInput(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 27:
		exit(0);
		break;
	case 'x':
		Xangle += 5.0;
		if (Xangle > 360.0) Xangle -= 360.0;
		glutPostRedisplay();

		break;
	case 'X':
		Xangle -= 5.0;
		if (Xangle < 0.0) Xangle += 360.0;
		glutPostRedisplay();
		break;
	case 'y':
		Yangle += 5.0;
		if (Yangle > 360.0) Yangle -= 360.0;
		//Face_rotate();
		glutPostRedisplay();
		break;
	case 'Y':
		Yangle -= 5.0;
		if (Yangle < 0.0) Yangle += 360.0;
		glutPostRedisplay();
		break;
	case 'z':
		Zangle += 5.0;
		if (Zangle > 360.0) Zangle -= 360.0;
		glutPostRedisplay();
		break;
	case 'Z':
		Zangle -= 5.0;
		if (Zangle < 0.0) Zangle += 360.0;
		glutPostRedisplay();
		break;
	case'c':
		change = 1;//0切换为平面 1切换到平滑
		glutPostRedisplay();
		break;
	case'C':
		change = 0;
		glutPostRedisplay();
		break;
	default:
		break;
	}
}

// Routine to output interaction instructions to the C++ window.
void printInteraction(void)
{
   std::cout << "Interaction:" << std::endl;
   std::cout << "Press x, X, y, Y, z, Z to turn the object." << std::endl;
}

// Main routine.
int main(int argc, char **argv)
{
   printInteraction();
   glutInit(&argc, argv);

   glutInitContextVersion(4, 3);
   glutInitContextProfile(GLUT_COMPATIBILITY_PROFILE);

   glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
   glutInitWindowSize(500, 500);
   glutInitWindowPosition(100, 100);
   glutCreateWindow("OBJmodelViewer.cpp");
   glutDisplayFunc(drawScene);
   glutIdleFunc(drawScene);
   glutReshapeFunc(resize);
   glutKeyboardFunc(keyInput);

   glewExperimental = GL_TRUE;
   glewInit();

   setup();

   glutMainLoop();
}

