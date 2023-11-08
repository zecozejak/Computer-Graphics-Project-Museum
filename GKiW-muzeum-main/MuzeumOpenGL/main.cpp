/*
Niniejszy program jest wolnym oprogramowaniem; mo¿esz go
rozprowadzaæ dalej i / lub modyfikowaæ na warunkach Powszechnej
Licencji Publicznej GNU, wydanej przez Fundacjê Wolnego
Oprogramowania - wed³ug wersji 2 tej Licencji lub(wed³ug twojego
wyboru) którejœ z póŸniejszych wersji.

Niniejszy program rozpowszechniany jest z nadziej¹, i¿ bêdzie on
u¿yteczny - jednak BEZ JAKIEJKOLWIEK GWARANCJI, nawet domyœlnej
gwarancji PRZYDATNOŒCI HANDLOWEJ albo PRZYDATNOŒCI DO OKREŒLONYCH
ZASTOSOWAÑ.W celu uzyskania bli¿szych informacji siêgnij do
Powszechnej Licencji Publicznej GNU.

Z pewnoœci¹ wraz z niniejszym programem otrzyma³eœ te¿ egzemplarz
Powszechnej Licencji Publicznej GNU(GNU General Public License);
jeœli nie - napisz do Free Software Foundation, Inc., 59 Temple
Place, Fifth Floor, Boston, MA  02110 - 1301  USA
*/

#define GLM_FORCE_RADIANS
#define GLM_FORCE_SWIZZLE
#define GLEW_STATIC

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <stdlib.h>
#include <stdio.h>
#include "shaderprogram.h"
#include "constants.h"
#include "lodepng.h"
#include "model.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>
#include <vector>
#include <unordered_map>

float speed_x = 0;
float speed_y = 0;
float aspectRatio = 1;
ShaderProgram* sp;
std::unordered_map<std::string, Model> models;

glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, -5.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
float lastX = 400, lastY = 300;
float yaw = -90.0f;
float pitch = -90.0f;
bool firstMouse = true;

GLuint readTexture(std::string filename) {
	GLuint tex;
	glActiveTexture(GL_TEXTURE0);

	//Wczytanie do pamiêci komputera
	std::vector<unsigned char> image;   //Alokuj wektor do wczytania obrazka
	unsigned width, height;   //Zmienne do których wczytamy wymiary obrazka
	//Wczytaj obrazek
	unsigned error = lodepng::decode(image, width, height, filename);

	//Import do pamiêci karty graficznej
	glGenTextures(1, &tex); //Zainicjuj jeden uchwyt
	glBindTexture(GL_TEXTURE_2D, tex); //Uaktywnij uchwyt
	//Wczytaj obrazek do pamiêci KG skojarzonej z uchwytem
	glTexImage2D(GL_TEXTURE_2D, 0, 4, width, height, 0,
		GL_RGBA, GL_UNSIGNED_BYTE, (unsigned char*)image.data());

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	return tex;
}

Model loadModel(std::string plik, std::string plik_tex) {
	Model model;
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(plik, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals);

	if (!scene) {
		std::cout << "Blad wczytywania pliku modelu" << std::endl;
		return model;
	}
	else std::cout << "Pobrano model" << std::endl;

	aiMesh* mesh;
	
	//body
	mesh = scene->mMeshes[0];
	for (int i = 0; i < mesh->mNumVertices; i++) {
		aiVector3D vertex = mesh->mVertices[i];
		model.modelVerts.push_back(glm::vec4(vertex.x, vertex.y, vertex.z, 1));

		aiVector3D normal = mesh->mNormals[i];
		model.modelNorms.push_back(glm::vec4(normal.x, normal.y, normal.z, 0));

		aiVector3D texCoord = mesh->mTextureCoords[0][i];
		model.modelTexCoords.push_back(glm::vec2(texCoord.x, texCoord.y));
	}

	for (int i = 0; i < mesh->mNumFaces; i++) {
		aiFace& face = mesh->mFaces[i];

		for (int j = 0; j < face.mNumIndices; j++) {
			model.modelIndices.push_back(face.mIndices[j]);
		}
	}
	model.tex0 = readTexture(plik_tex);
	return model;
}

//Procedura obs³ugi b³êdów
void error_callback(int error, const char* description) {
	fputs(description, stderr);
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	const float cameraSpeed = 0.05f;
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		cameraPos += cameraSpeed * glm::vec3(cameraFront.x, 0.0f, cameraFront.z);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		cameraPos -= cameraSpeed * glm::vec3(cameraFront.x, 0.0f, cameraFront.z);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	float sensitivity = 0.1f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;

	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	glm::vec3 direction;
	direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	direction.y = sin(glm::radians(pitch));
	direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraFront = glm::normalize(direction);
}

void windowResizeCallback(GLFWwindow* window, int width, int height) {
	if (height == 0) return;
	aspectRatio = (float)width / (float)height;
	glViewport(0, 0, width, height);
}

void readModelsFromFiles() {
	models.insert(std::make_pair<std::string, Model>("Scaniverse", loadModel("Scaniverse.obj", "Scaniverse.png")));
	models.insert(std::make_pair<std::string, Model>("Monalisa", loadModel("Mona Lisa.obj", "PaintingMonalisa.png")));
}

//Procedura inicjuj¹ca
void initOpenGLProgram(GLFWwindow* window) {
	//************Tutaj umieszczaj kod, który nale¿y wykonaæ raz, na pocz¹tku programu************
	glClearColor(0, 0, 0, 1);
	glEnable(GL_DEPTH_TEST);
	glfwSetWindowSizeCallback(window, windowResizeCallback);
	glfwSetKeyCallback(window, keyCallback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window, mouse_callback);
	sp = new ShaderProgram("v_simplest.glsl", NULL, "f_simplest.glsl");
	readModelsFromFiles();
}


//Zwolnienie zasobów zajêtych przez program
void freeOpenGLProgram(GLFWwindow* window) {
	//************Tutaj umieszczaj kod, który nale¿y wykonaæ po zakoñczeniu pêtli g³ównej************
	delete sp;
}

void drawObject(Model model, glm::vec3 scale, glm::vec3 translate, float rotate, glm::vec3 hook) {
	glm::mat4 M = glm::mat4(1.0f);
	M = glm::scale(M, scale);
	M = glm::translate(M, translate);
	M = glm::rotate(M, rotate, hook);
		
	glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(M));

	glEnableVertexAttribArray(sp->a("vertex"));
	glVertexAttribPointer(sp->a("vertex"), 4, GL_FLOAT, false, 0, model.modelVerts.data());

	glEnableVertexAttribArray(sp->a("normal"));
	glVertexAttribPointer(sp->a("normal"), 4, GL_FLOAT, false, 0, model.modelNorms.data());

	glEnableVertexAttribArray(sp->a("texCoord0"));
	glVertexAttribPointer(sp->a("texCoord0"), 2, GL_FLOAT, false, 0, model.modelTexCoords.data());

	glUniform1i(sp->u("textureMap0"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, model.tex0);

	glDrawElements(GL_TRIANGLES, model.modelIndices.size(), GL_UNSIGNED_INT, model.modelIndices.data());

	glDisableVertexAttribArray(sp->a("vertex"));
	glDisableVertexAttribArray(sp->a("color"));
	glDisableVertexAttribArray(sp->a("normal"));
	glDisableVertexAttribArray(sp->a("texCoord0"));
}

//Procedura rysuj¹ca zawartoœæ sceny
void drawScene(GLFWwindow* window, float angle_x, float angle_y) {
	//************Tutaj umieszczaj kod rysuj¹cy obraz******************l
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4 V = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp); // macierz widoku
	glm::mat4 P = glm::perspective(glm::radians(50.0f), 1.0f, 1.0f, 50.0f); // maczierz perspektywy (FOV)

	sp->use();
	glUniformMatrix4fv(sp->u("P"), 1, false, glm::value_ptr(P));
	glUniformMatrix4fv(sp->u("V"), 1, false, glm::value_ptr(V));
	
	///******* W tym miejscu wypisujemy pozycje wszystkich obiektów ***********///
	drawObject(models["Scaniverse"], glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	drawObject(models["Monalisa"], glm::vec3(0.01f, 0.01f, 0.01f), glm::vec3(0.0f, 0.0f, 0.0f), glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	///************************************************************************///

	glfwSwapBuffers(window); //Przerzuæ tylny bufor na przedni
}


int main(void)
{
	GLFWwindow* window; //WskaŸnik na obiekt reprezentuj¹cy okno

	glfwSetErrorCallback(error_callback);//Zarejestruj procedurê obs³ugi b³êdów

	if (!glfwInit()) { //Zainicjuj bibliotekê GLFW
		fprintf(stderr, "Nie mo¿na zainicjowaæ GLFW.\n");
		exit(EXIT_FAILURE);
	}

	window = glfwCreateWindow(1920, 1080, "OpenGL", NULL, NULL);  //Utwórz okno 500x500 o tytule "OpenGL" i kontekst OpenGL.

	if (!window) //Je¿eli okna nie uda³o siê utworzyæ, to zamknij program
	{
		fprintf(stderr, "Nie mo¿na utworzyæ okna.\n");
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window); //Od tego momentu kontekst okna staje siê aktywny i polecenia OpenGL bêd¹ dotyczyæ w³aœnie jego.
	glfwSwapInterval(1); //Czekaj na 1 powrót plamki przed pokazaniem ukrytego bufora

	if (glewInit() != GLEW_OK) { //Zainicjuj bibliotekê GLEW
		fprintf(stderr, "Nie mo¿na zainicjowaæ GLEW.\n");
		exit(EXIT_FAILURE);
	}

	initOpenGLProgram(window); //Operacje inicjuj¹ce

	//G³ówna pêtla
	float angle_x = 0;
	float angle_y = 0;
	glfwSetTime(0); //Zeruj timer
	while (!glfwWindowShouldClose(window)) //Tak d³ugo jak okno nie powinno zostaæ zamkniête
	{
		angle_x += speed_x * glfwGetTime();
		angle_y += speed_y * glfwGetTime();
		glfwSetTime(0); //Zeruj timer
		drawScene(window, angle_x, angle_y); //Wykonaj procedurê rysuj¹c¹
		glfwPollEvents(); //Wykonaj procedury callback w zaleznoœci od zdarzeñ jakie zasz³y.
	}

	freeOpenGLProgram(window);

	glfwDestroyWindow(window); //Usuñ kontekst OpenGL i okno
	glfwTerminate(); //Zwolnij zasoby zajête przez GLFW
	exit(EXIT_SUCCESS);
}
