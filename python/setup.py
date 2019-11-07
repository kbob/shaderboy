import setuptools

try:
    with open('../README.md') as f:
        long_description = f.read()
except FileNotFoundError:
    # Pip runs this from some other directory and I don't care.
    long_description = '???'

setuptools.setup(
    name='shade-kbob',
    version='0.0.1',
    author_email='kbob@jogger-egg.com',
    description='GL shaders for LED cubes',
    long_description=long_description,
#    long_description_content_type='text/markdown',
    url='https://github.com/kbob/shaderboy',
    packages=setuptools.find_packages(),
    python_requires='>=3.5',
)
