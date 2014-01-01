
#include <GL/gl.h>
#include <GL/glu.h>
#include <iostream>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <obj/obj.hpp>

int main(int argc, char** argv)
{
	if (argc > 1)
	{
		sf::RenderWindow win(sf::VideoMode(640,480),"Millenium");
		bool running = true;
		Obj* myobj = new Obj(std::string(argv[1]));
		float rotate_x=0,rotate_y=0;
		glEnable(GL_DEPTH_TEST);
		while (running)
		{
			// gestion des évènements
			sf::Event event;
			while (win.pollEvent(event))
			{
				if (event.type == sf::Event::Closed)
				{
					// on stoppe le programme
					running = false;
				}
				else if (event.type == sf::Event::Resized)
				{
					// on ajuste le viewport lorsque la fenêtre est redimensionnée
					glViewport(0, 0, event.size.width, event.size.height);
				}
			}

			// effacement les tampons de couleur/profondeur
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// Reset transformations
			glLoadIdentity();
 
			glRotatef(rotate_x,1,0,0);
			glRotatef(rotate_y,0,1,0);
			myobj->render();

			win.display();
			rotate_x+=0.15;
			rotate_y+=0.2;
			sf::sleep(sf::milliseconds(10));
		}
	}
	else
	{
		std::cout << "no filenames..." << std::endl;
	}

  
  return 0;
}