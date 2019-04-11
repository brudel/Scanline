#include <iostream>
#include <fstream>
#include <tuple>
#include <vector>
#include <algorithm>

// OpenGL library
#include <GL/glut.h>
#include <GL/glu.h>
#include <GL/gl.h>

using namespace std;

/**********  Classes   **********/
class Edge{
    public:
        GLint minX, minY, maxX, maxY;
        GLfloat xStartPos, M;

    Edge(tuple<GLint, GLint> vertex1, tuple<GLint, GLint> vertex2){
        this->maxX = max(get<0>(vertex1), get<0>(vertex2));
        this->maxY = max(get<1>(vertex1), get<1>(vertex2));
        this->minX = min(get<0>(vertex1), get<0>(vertex2));
        this->minY = min(get<1>(vertex1), get<1>(vertex2));
        
        // Define starting x value
        if(min(get<1>(vertex1), get<1>(vertex2)) == get<1>(vertex1)){
            this->xStartPos = get<0>(vertex1);
        }else{
            this->xStartPos = get<0>(vertex2);
        }

        // Calculates the M of edge
        this->M = static_cast<GLfloat>(static_cast<GLfloat>(get<1>(vertex1) - get<1>(vertex2))) / 
        static_cast<GLfloat>((get<0>(vertex1) - get<0>(vertex2)));
    }
};

// Stores color data pixel.
class RGBType {
    public:
        GLfloat r, g, b;
};


/**********  Globals   **********/
GLint scanline;                         // Value of Y of the scanline
bool DRAWING;                           // Controls user input
vector<tuple<GLint, GLint>> points;     // All vertexes of the polygon
vector<Edge> allEdges;                  // Every Edge of the polygon
vector<Edge> activeEdges;               // Edges in process of scanline
RGBType *pixels;                        // All pixels of the screen


/**********  Functions   **********/
void setInicialConfig(){
    //Set background color
    glClearColor(1.0, 1.0, 1.0, 0.0);
    
    //Initialize camera
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, 500, 500, 0.0, 0.0, 1.0);
    
    //Initialize RGB array
    pixels = new RGBType[500*500];
    
    // Set everything to black initially.
    for (int i = 0; i < 500*500; i++) {
        pixels[i].r = 0;
        pixels[i].g = 0;
        pixels[i].b = 0;
    }
    
    DRAWING = false;
}

// Sort by minY, if equal sort by minX
bool sortByMinY(const Edge& edge1, const Edge& edge2){
    if (edge1.minY != edge2.minY)
        return (edge1.minY < edge2.minY);
    return (edge1.minX < edge2.minX);
}

// Sort by current x values
bool sortByXPos(const Edge& edge1, const Edge& edge2){
    return (edge1.xStartPos < edge2.xStartPos);
}

// Draw pixels from (x1, scanline) to (x2, scanline).
void draw(GLfloat x1, GLfloat x2) {
    // Round points, do not do floor!
    int point1 = roundf(x1);
    int point2 = roundf(x2);
    int count = 0;
    
    // Changes color of the pixels to white
    for (int i = ((500 * (500 - scanline)) + point1); i < ((500 * (500 - scanline)) + point2); i++) {
        pixels[i].r = 1;
        pixels[i].b = 1;
        pixels[i].g = 1;
        glutPostRedisplay();
    }
}

// Update the current X values of the active edges
void updateXValues(){
    for (vector<Edge>::iterator it = activeEdges.begin(); it < activeEdges.end(); it++) {
        it->xStartPos += (1/it->M); //  x = x + 1/m
    }
}

// Add edges from allEdges to activeEdges if scanline reaches minY
void addActiveEdges(){
    for (vector<Edge>::iterator it = allEdges.begin(); it < allEdges.end(); it++) {
        if (it->minY == scanline) {
            activeEdges.push_back(*it);
        }else if (it->minY > scanline) {
            return; // optimize time
        }
    }
}

// Remove edge if maxY == scanline
void removeActiveEdges(){
    for (vector<Edge>::iterator it = activeEdges.begin(); it < activeEdges.end(); ) {
        if (it->maxY == scanline) {
            activeEdges.erase(it);
        }else {
            it++;  // Prevents seg fault
        }
    }
}

// Sort Edges to begin scanline
void sortAllEdges() {
    sort(allEdges.begin(), allEdges.end(), sortByMinY);  // Sort Edges
    // Iterate over all edges
    for(vector<Edge>::iterator it = allEdges.begin(); it < allEdges.end(); it++){
        // Remove horizontal edges
        if(it->M == 0){
            allEdges.erase(it);
        }
    }
}

void scanlineFill() {
    while(activeEdges.size() != 0){
        // Draw pixels from edges
        for(vector<Edge>::iterator it = activeEdges.begin(); it < activeEdges.end(); it++){
            draw(it->xStartPos, (it+1)->xStartPos);
            it++;
        }

        scanline++; // updates scanline to next Y
        removeActiveEdges();
        updateXValues();
        addActiveEdges();
        
        sort(activeEdges.begin(), activeEdges.end(), sortByXPos); // Sort active edges by x value
        
        glutPostRedisplay();  // Update screen
    }
}

// Function for menu
void menu(int id) {
    switch (id) {
        // If chooses to draw polygon
        case 1:
            // If has at least three points
            if(allEdges.size() > 1){
                // Add final edge of polygon
                Edge newEdge(points.at(0), points.at(points.size()-1));
                allEdges.push_back(newEdge);

                // Do initial config to start scanline
                sortAllEdges();
                scanline = allEdges.at(0).minY; // Set scanline to smallest Y
                addActiveEdges();

                DRAWING = true; // User cannot draw anymore

                glutPostRedisplay();  // Update display

                scanlineFill();  // Do scanline fill 
            }
            break;
    }
    glutPostRedisplay();
}

// Function for Keyboard keys
void keyboard(unsigned char key, int xmouse, int ymouse) {
    switch (key) {
        // If Esc is pressed, exit program
        case 27:
            free(pixels); // Free memory
            glutDestroyWindow(glutGetWindow());
            exit(0);
            break;
    }
    glutPostRedisplay();
}

// Function for mouse click
void mouse_down(int button, int state, int x, int y) {
    switch (button) {
        // If is left click
        case GLUT_LEFT_BUTTON:
            // See if user can draw
            if(state == GLUT_DOWN && !DRAWING) {
                points.push_back(tuple<GLint, GLint>(x, y));  // Save the x,y coordenates
                // If there are more then one points, creates edge
                if(points.size() > 1) {
                    // Create edge from the last two points
                    Edge newEdge(points.at(points.size()-2), points.at(points.size()-1));
                    allEdges.push_back(newEdge);
                }
                glutPostRedisplay();
            }
    }
    glutPostRedisplay();
}

// Function for display
void display(){
    // Look for OpenGL errors
    GLenum err_code;
    do {
        err_code = glGetError();
        if (err_code != GL_NO_ERROR)
            printf("Error: %s\n", gluErrorString(err_code));
    } while (err_code != GL_NO_ERROR);
    
    //Clear buffer data
    glClear(GL_COLOR_BUFFER_BIT);
    
    //Draw pixels, using pixels array
    glDrawPixels(500, 500, GL_RGB, GL_FLOAT, pixels);
    
    // Defines the size and color of the point
    glPointSize(5);
    glColor3f(1.0, 1.0, 1.0);
    
    // Draw screen
    if (!DRAWING) {
        for (int i = 0; i < points.size(); i++) {
            glBegin(GL_POINTS);
            glVertex2f(get<0>(points.at(i)), get<1>(points.at(i)));
            glEnd();
        }
    }
    
    // Draws the final screen 
    if(DRAWING) {
        glDrawPixels(500, 500, GL_RGB, GL_FLOAT, pixels);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glBegin(GL_POLYGON);
        for (int i = 0; i < points.size(); i++) {
            glVertex2f(get<0>(points.at(i)), get<1>(points.at(i)));
        }
        glEnd();
    }
    
    //Flush data
    glFlush();
}

int main(int argc, char** argv){
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);

    // Create a window in the middle of screen 
    glutInitWindowSize(500, 500);
    glutInitWindowPosition((glutGet(GLUT_SCREEN_WIDTH)-500)/2, (glutGet(GLUT_SCREEN_HEIGHT)-500)/2);

    glutCreateWindow("Scan Line Fill"); // Window Title

    setInicialConfig(); // Set inicial config

    // Define Behaviour of screen
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse_down);

    // Create right click menu
    glutCreateMenu(menu);
    glutAddMenuEntry("Draw Polygon", 1);
    glutAttachMenu(GLUT_RIGHT_BUTTON);
    
    glutMainLoop();  // Main loop is here

    return 0;
}