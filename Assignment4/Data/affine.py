# program to experiment with affine transformations
import cv2
import numpy as np

# cv2.imshow('img',dst)
# cv2.waitKey(0)
# cv2.destroyAllWindows()

def list_commands():
	print()
	print('available commands:')
	print('  load        load an image')
	print('  scale       rescale the image')
	print('  translate   generate translation matrix')
	print('  rotate      generate rotation matrix')
	print('  affine      perform affine transformation')
	print('  quit        quit the program')

def show_matrix(M):
	print(M)

def close_windows():
	cv2.waitKey(0)
	cv2.destroyAllWindows()
	cv2.waitKey(1) 

def main():
	print('********************************************')
	print('* demo program opencv affine transformations')
	print('********************************************')

	list_commands()
	cmd = ' '
	while cmd[0] != 'q':
		cmd = input('\ncommand: ')
		if cmd == '':
			continue
		elif cmd[0] == 'l':
			fname = input("file name ('original.png'): ")
			if fname == '':
				fname = 'original.png'
			try:
				img = cv2.imread(fname, 0)
				rows,cols = img.shape
				cv2.imshow('loaded image', img)
				close_windows()
			except:
				print('filed to load image', fname)
				list_commands()
		elif cmd[0] == 's':
			cx = float(input('scaling factor x: '))
			cy = float(input('scaling factor y: '))
			M = np.float32([[cx, 0, 0], [0, cy, 0]])
			print('matrix = ')
			print(M)
		elif cmd[0] == 't':
			dx = float(input('delta x: '))
			dy = float(input('delta y: '))
			M = np.float32([[1, 0, dx], [0, 1, dy]])
			print('matrix = ')
			print(M)
		elif cmd[0] == 'r':
			angle = float(input('angle (in degrees): '))
			xc = float(input('x of rotation center: '))
			yc = float(input('y of rotation center: '))
			M = cv2.getRotationMatrix2D((xc, yc), angle, 1)
			print('matrix = ')
			print(M)
		elif cmd[0] == 'm':
			M = np.float32([[1,0,100],[0,1,50], [0, 0, 1]])
			show_matrix(M)
		elif cmd[0] == 'a':
			dst = cv2.warpAffine(img,M,(cols,rows))
			cv2.imshow('original image', img)
			cv2.imshow('transformed image', dst)
			close_windows()
		elif cmd[0] == 'q':
			print('leaving program ...')
			print()

if __name__ == '__main__':
	main()