# Matrix addition in vyne
# I was thinking how to do this for the whole night lmao, honestly worth it though
# I'll probably try to do the same with nested while loops too, just not yet.

sub matrix_add(a, b) {

    result :: Array = [];

    row_index = 0;

    through a -> collect {

        rowA = a[row_index];
        rowB = b[row_index];

        new_row = [];

        col_index = 0;

        through rowA -> collect {

            value = rowA[col_index] + rowB[col_index];

            new_row.push(value);

            col_index = col_index + 1;
        };

        result.push(new_row);

        row_index = row_index + 1;
    };

    return result;
}

a :: Array = [
    [1, 2],
    [3, 3] # This works for any kind of matrix, although larger matrices are much slower to calculate
];

b :: Array = [
    [5, 6],
    [7, 8],
];

log(matrix_add(a, b));