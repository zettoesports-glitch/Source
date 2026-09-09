#version 330 core
uniform vec4 u_BodyLight;

// [Genesys][2026-09-08] Forca da sombra na hora que esta valendo.
// Sol a pino projeta sombra dura; de madrugada nao ha sol, e o que sobra e quase
// nada. Multiplicar aqui alcanca TODOS os chamadores de uma vez, cada um com o
// proprio alfa base, em vez de mexer em cada Color4f espalhado pelo cliente.
// Sem ciclo o cliente manda 1.0 e o desenho sai identico ao de antes.
uniform float u_ShadowStrength;

out vec4 FragColor;

void main()
{
    FragColor = vec4(u_BodyLight.rgb, u_BodyLight.a * u_ShadowStrength);
}
