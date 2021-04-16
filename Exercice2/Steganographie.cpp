#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <stdio.h>
#include <iostream>
#include <fstream>

/*
Le code ci-dessous implémente la stéganographie, comme expliquée dans l'article de Dasgupta et al.: 
"HASH BASED LEAST SIGNIFICANT BIT TECHNIQUE FOR VIDEO STEGANOGRAPHY(HLSB)"
*/
using namespace cv;
using namespace std;

/*
Cette structure représente les paramètres nécessaires au décodage
*/
struct Encoding_info
{
	int original_width;
	int original_height;
	int lsb_bits;
};

/*
Cette fonction permet de transformer un int en string binaire à 8 caractères: 5 devient donc "00000101"
*/
string intToBit(int n)
{
    string r;
    //La ligne suivante permet d'écrire l'int en binaire
    while(n!=0) {r=(n%2==0 ?"0":"1")+r; n/=2;}
    //Les lignes suivantes permettent de rajouter des 0 au début de la chaîne pour avoir 8 caractères
    int initial_length = r.length();
    if (initial_length<8){
    	for (int z=0; z<8-initial_length; z++)
    		r = "0"+r;
    }
    return r;
}

/*
Cette fonction permet de cacher l'image opencv.jpg à l'intérieur de l'image lena.png
*/
void encode(){
	// On récupère les images et on crée l'image finale à partir de l'image "réceptacle"
	Mat to_encode = imread("../data/opencv.jpg");
	Mat encodant = imread("../data/lena.png");
	Mat output_img = encodant.clone();

	//On garde en mémoire les dimensions de chaque image, qui seront réutilisées plus loin
	int to_encode_w = to_encode.size().width;
	int to_encode_h = to_encode.size().height;
	int encodant_w = encodant.size().width;
	int encodant_h = encodant.size().height;

	//On passe l'image à encoder en grayscale
	Mat to_encode_gray;
	cvtColor(to_encode, to_encode_gray, COLOR_BGR2GRAY);

	//Il est nécessaire de sauvegarder les dimensions de l'image d'origine, ainsi que le nombre de LSb utilisés
	Encoding_info info;
	info.lsb_bits = 4;
	info.original_width = to_encode_w;
	info.original_height = to_encode_h;

	//On affiche les images d'origine
	imshow("Image a encoder", to_encode_gray);
	imshow("Image receptacle", encodant);

	//On boucle sur les pixels de l'image à encoder
	int pixel_nb = 0;
	for (int i=0; i<to_encode_w; i++){
		for (int j=0; j<to_encode_h; j++){
			//Pour chaque pixel, on calcule les coordonnées du pixel correspondant dans l'image réceptacle
			pixel_nb+=1;
			int x = pixel_nb/encodant_w;
			int y = pixel_nb-(x*encodant_w);
			if (y>0)
				y-=1;
			
			//On convertit la valeur de notre pixel à encoder en string binaire
			int input_pixel_value = (int)to_encode_gray.at<uchar>(j,i);
			string input_binary = intToBit(input_pixel_value);

			//On fait la même chose pour les trois valeurs (BGR) de notre image cible
			Vec3b output_pixel_value = output_img.at<Vec3b>(Point(y,x));
			string b_binary = intToBit(output_pixel_value[0]);
			string g_binary = intToBit(output_pixel_value[1]);
			string r_binary = intToBit(output_pixel_value[2]);

			//Cet offset dépend du nombre de LSB choisi: il permet de calculer la position k dans la chaine de caractères
			int offset = 8-info.lsb_bits-1;

			//Pour chaque bit p de notre valeur de pixel à encoder, on calcule la position k qui itère
			//Puis on répartit nos bit en 3,3,2 sur les valeurs binaires de rouge, vert et bleu respectivement
			for (int p=1; p<=8; p++){
				int k = p%info.lsb_bits;
				if (k==0)
					k=info.lsb_bits;

				if (p<4)
					r_binary[offset+k] = input_binary[p-1];
				else if (p>6)
					b_binary[offset+k] = input_binary[p-1];
				else
					g_binary[offset+k] = input_binary[p-1];
			}

			//On reconvertit nos trois nouvelles valeurs binaires en int
			output_pixel_value[0] = (int)stoi(b_binary, nullptr, 2);
			output_pixel_value[1] = (int)stoi(g_binary, nullptr, 2);
			output_pixel_value[2] = (int)stoi(r_binary, nullptr, 2);

			//On remplace le pixel par sa nouvelle valeur contenant le pixel à encoder
			output_img.at<Vec3b>(Point(y,x)) = output_pixel_value;
		}
	}

	//On affiche l'image encodée et on sauvegarde nos resultats ainsi que nos informations d'encoding
	cout << "Saving encoded_image.png and encoding_info.txt in current folder" << endl;
	imshow("Image encodée", output_img);
	imwrite("./encoded_image.png", output_img);
    ofstream fout("./encoding_info.txt");
    fout << info.original_width << ' ' << info.original_height << ' ' << info.lsb_bits;
    fout.close();
}

/*
Ce code permet de décoder l'image cachée
*/
void decode(){
	//On récupère notre image encodée et on l'affiche
	Mat encoded = imread("./encoded_image.png", IMREAD_UNCHANGED);
	imshow("Image encodée", encoded);

	//On récupère également nos informations d'encoding, soit la dimension de l'image d'origine et notre nombre de LSB
    Encoding_info info;
    ifstream fin("./encoding_info.txt");
    fin >> info.original_width >> info.original_height >> info.lsb_bits;
    fin.close();

    //On crée la matrice de l'image (message) cachée
    Mat message = Mat::zeros(Size(info.original_width, info.original_height), CV_8UC1);

    //On boucle sur les pixels de l'image à reconstituer
    int pixel_nb = 0;
	for (int i=0; i<info.original_width; i++){
		for (int j=0; j<info.original_height; j++){
			//Pour chaque pixel, on calcule les coordonnées du pixel correspondant dans l'image encodée
			pixel_nb+=1;
			int x = pixel_nb/encoded.size().width;
			int y = pixel_nb-(x*encoded.size().width);
			if (y>0)
				y-=1;

			//On récupère et convertit en binaire les valeurs BGR de l'image encodée
			Vec3b input_pixel_value = encoded.at<Vec3b>(Point(y,x));
			string b_binary = intToBit(input_pixel_value[0]);
			string g_binary = intToBit(input_pixel_value[1]);
			string r_binary = intToBit(input_pixel_value[2]);

			//L'offset nous permet là aussi de récupérer les pixels aux positions k sur la partie de LSB
			int offset = 8-info.lsb_bits-1;

			//On initialise notre valeur binaire à récuperer
			string binary_pixel = "";

			//Pour chacun de 8 bits, on calcule la position k du bit à récuperer
			//dans les pixel de rouge, vert et de bleu
			for (int p=1; p<=8; p++){
				int k = p%info.lsb_bits;
				if (k==0)
					k=info.lsb_bits;

				if (p<4)
					binary_pixel += r_binary[offset+k];
				else if (p>6)
					binary_pixel += b_binary[offset+k];
				else
					binary_pixel += g_binary[offset+k];
			}

			//On reconvertit notre pixel binaire récupéré en int
			int pixel_value = (int)stoi(binary_pixel, nullptr, 2);

			//Et on replace notre pixel dans l'image décodée
			message.at<uchar>(j, i) = pixel_value;

		}
	}

	//Enfin, on affiche notre image ou message décodé
	imshow("Message décodé", message);
}

int main(){
	encode();
	waitKey(0);
	decode();
	waitKey(0);
	return 0;
}