float max2(float x, float y) 
{
	if(x >= y)
		return x;
	else
		return y;
}

float min2(float x, float y) 
{
	if(x >= y)
		return y;
	else
		return x;
}

float max3(float x, float y, float z) 
{
	if (x>=y && x>=z)
		return x;
	else if(y>=x && y>=z)
		return y;
	else
		return z;
}

float min3(float x, float y, float z) 
{
	if(x<=y && x<=z)
		return x;
	else if(y<=x && y<=z)
		return y;
	else
		return z;
}

float max4(float x, float y, float z, float w)
{
	return max2( max2(x, y), max2(z, w));
}

float min4(float x, float y, float z, float w)
{
	return min2( min2(x, y), min2(z, w));
}

float mid3(float x, float y, float z)
{
	if(y<x && x<z)
		return x;
	else if(z<x && x<y)
		return  x;
	if(x<y && y<z)
		return y;
	else if(z<y && y<x)
		return  y;
	else
		return z;
}