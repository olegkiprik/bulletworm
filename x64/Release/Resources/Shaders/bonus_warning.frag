uniform sampler2D texture;
uniform float time;

void main()
{
	float value = (3.0 + sin(5.0*time)) / 5.0;
	vec4 color = gl_Color * texture2D(texture, gl_TexCoord[0].xy);
	gl_FragColor = vec4(color.rgb*value, color.a);
}