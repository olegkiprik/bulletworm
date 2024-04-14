uniform sampler2D texture;
uniform float time;

void main()
{
	float value = (3.0+sin(time))/4.0;
	gl_FragColor = gl_Color * texture2D(texture, gl_TexCoord[0].xy).bgra * vec4(1,1,1,value);
}