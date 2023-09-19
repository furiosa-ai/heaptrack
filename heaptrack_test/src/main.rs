use itertools::Itertools;
use ndarray::{ArrayViewD, ShapeBuilder, Zip};

fn main() {
    let input_sizes = vec![2, 16, 4, 8, 30, 8, 4, 3, 3, 8];
    let input_volume: usize = input_sizes.iter().copied().product();
    dbg!(&input_volume);
    let vs = vec![1u8; input_volume];

    let array1 = unsafe {
        ArrayViewD::from_shape_ptr(
            input_sizes
                .clone()
                .strides(vec![0, 8, 276480, 34560, 384, 0, 0, 128, 11520, 1]),
            vs.as_ptr(),
        )
    };

    let array2 = unsafe {
        ArrayViewD::from_shape_ptr(
            input_sizes
                .clone()
                .strides(vec![73728, 0, 0, 0, 0, 8, 576, 192, 64, 1]),
            vs.as_ptr(),
        )
    };

    let chunk_shape = vec![1, 1, 4, 8, 30, 8, 4, 3, 3, 8];

    let output = Zip::from(array1.exact_chunks(chunk_shape.clone()))
        .and(array2.exact_chunks(chunk_shape))
        .par_map_collect(|a, b| {
            a.iter()
                .zip(b)
                .map(|(x, y)| x - y)
                .chunks(32)
                .into_iter()
                .map(|chunk| 1usize)
                .sum::<usize>()
        });
    dbg!(output.into_iter().sum::<usize>());
}
