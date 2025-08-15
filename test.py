import fire
from PIL import Image


def test():
    # Set dimensions
    width, height = 240, 320

    # Create a white RGB image
    image = Image.new('RGB', (width, height), color='red')

    # Save the image as JPEG
    # image.save('baseline_image.jpg', 'JPEG', quality=85)
    image.save('image_baseline.jpg', format='JPEG', quality=85, optimize=False, progressive=False)


    image = Image.new('RGB', (width, height), color='blue')
    image.save('image.jpg', format='JPEG', quality=85)

def check(image):
    img = Image.open(image)
    print("Format:", img.format)
    print("Mode:", img.mode)
    print("Size:", img.size)
    print("Info:", img.info)
    print(img.__dict__)

if __name__ == '__main__':
    fire.Fire({
        "check": check,
        "test": test,
    })