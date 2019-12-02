PlayerGrid(String name) 
	Input strng that is used to name the window

display() 
	Displays grid without ships

displayWithBoats() 
	Displays grid with ships

shoot(int status, int x, int y) 
	status tells if shot is made, 0 = no shot was made, 1 = hit, 2 = miss
	x and y start from 0

scout(bool status, int x, int y) 
	status tells if a boat is there
	x and y start from 0

addBoat(int size, int locationX, int locationY, bool rotated) 
	add a boat to a grid
	rotation: false is vertical, true is horizontal