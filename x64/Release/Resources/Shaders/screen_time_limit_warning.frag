uniform sampler2D texture;
uniform float time;

void main()
{
	float value = (3.0+sin(time))/4.0;
	vec4 color = gl_Color * texture2D(texture, gl_TexCoord[0].xy);
	gl_FragColor = vec4(color.r*value,color.g*value,color.b*value,color.a) * vec4(1, 0.5,0.5,1);
}