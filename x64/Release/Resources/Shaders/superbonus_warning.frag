uniform sampler2D texture;
uniform float time;

void main()
{
	float value = (3.5+sin(5.0*time))/5.0;
	gl_FragColor = gl_Color * texture2D(texture, gl_TexCoord[0].xy).brga * vec4(1,1,1,value);
}