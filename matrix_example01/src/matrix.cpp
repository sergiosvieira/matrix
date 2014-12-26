﻿/*
 * matrix.cpp
 *
 *  Created on: 2014. 12. 23.
 *      Author: asran
 */

#include "matrix.h"
#include "matrix_error.h"
#include <stdio.h>
#include <string.h>
#include <new>

Matrix::Matrix	(	void	)
:mMatrix(NULL),
 mCol(0),
 mRow(0)
{

}

Matrix::~Matrix	(	void	)
{
	deallocMatrix();
}

bool	Matrix::allocMatrix		(	size_t		col,
										size_t		row
									)
{
	bool	ret	=	false;

	do
	{
		try
		{
			mMatrix	=	new int[col*row];
			if( mMatrix == NULL )
			{
				break;
			}
		}
		catch	(	std::bad_alloc&	exception	)
		{
			exception	=	exception;
			break;
		}

		memset(mMatrix, 0, col*row*sizeof(elem_t));

		mCol	=	col;
		mRow	=	row;

		ret	=	true;
	}while(0);

	return	ret;
}

void	Matrix::deallocMatrix	(	void	)
{
	delete[]	mMatrix;
	mMatrix	=	NULL;
}

void		Matrix::setValue			(	size_t		col,
											size_t		row,
											elem_t		value
										)
{
	chkBound(col, row);

	mMatrix[(col * mRow) + row]	=	value;
}

elem_t		Matrix::getValue			(	size_t		col,
											size_t		row
										)
{
	int		value	=	0;

	chkBound(col, row);

	value	=	mMatrix[(col * mRow) + row];

	return	value;
}

void		Matrix::equal				(	Matrix*	matrix		)
{
	chkSameSize(matrix);

	memcpy	(	mMatrix,
				matrix->mMatrix,
				this->getCol() * this->getRow() * sizeof(elem_t)
			);
}

void		Matrix::Add				(	Matrix*	operand,
											Matrix*	result
										)
{
	chkSameSize(operand);
	chkSameSize(result);

	for(size_t col=0;col<getCol();col++)
	{
		for(size_t row=0;row<getRow();row++)
		{
			result->setValue	(	col,
									row,
									this->getValue(col, row) + operand->getValue(col, row)
								);
		}
	}
}

void		Matrix::Sub				(	Matrix*	operand,
											Matrix*	result
										)
{
	chkSameSize(operand);
	chkSameSize(result);

	for(size_t col=0;col<getCol();col++)
	{
		for(size_t row=0;row<getRow();row++)
		{
			result->setValue	(	col,
									row,
									this->getValue(col, row) - operand->getValue(col, row)
								);
		}
	}
}

void		Matrix::Multiply			(	Matrix*	operand,
											Matrix*	result
										)
{
	if( ( this->getCol() != operand->getRow() ) ||
		( this->getRow() != operand->getCol() ) )
	{
		throw	matrix::ErrMsg::CreateErrMsg("연산 대상의 행렬 크기가 올바르지 않습니다.");
	}

	if( ( this->getCol() != result->getCol() ) ||
		( operand->getRow() != result->getRow() ) )
	{
		throw	matrix::ErrMsg::CreateErrMsg("결과 저장 행렬의 크기가 올바르지 않습니다.");
	}

	for(size_t col=0;col<result->getCol();col++)
	{
		for(size_t row=0;row<result->getRow();row++)
		{
			int value	=	0;
			for(size_t x=0;x<this->getRow();x++)
			{
				value	+=	this->getValue(col, x) * this->getValue(x, row);
			}
			result->setValue(col, row, value);
		}
	}
}

void		Matrix::Multiply			(	elem_t		operand,
											Matrix*	result
										)
{
	chkSameSize(result);

	for(size_t col=0;col<getCol();col++)
	{
		for(size_t row=0;row<getRow();row++)
		{
			result->setValue	(	col,
									row,
									this->getValue(col, row) * operand
								);
		}
	}
}


void		Matrix::chkBound			(	size_t		col,
											size_t		row
										)
{
	if( ( col >= mCol ) ||
		( row >= mRow ) )
	{
		throw	matrix::ErrMsg::CreateErrMsg("범위를 넘어서는 참조입니다.");
	}
}

void		Matrix::chkSameSize		(	Matrix*	matrix		)
{
	if( (this->getCol() != matrix->getCol()) ||
		(this->getRow() != matrix->getRow()) )
	{
		throw	matrix::ErrMsg::CreateErrMsg("크기가 다른 행렬입니다.");
	}
}

Matrix*	Matrix::createMatrix		(	size_t		col,
											size_t		row
										)
{
	Matrix*	newMatrix	=	NULL;

	do
	{
		try
		{
			newMatrix	=	new Matrix();
			if( newMatrix == NULL )
			{
				break;
			}
		}
		catch	(	std::bad_alloc&	exception	)
		{
			exception	=	exception;
			break;
		}

		if( newMatrix->allocMatrix(col, row) == false )
		{
			delete		newMatrix;
			newMatrix	=	NULL;
			break;
		}

	}while(0);

	return	newMatrix;
}

Matrix*	Matrix::createMatrix		(	Matrix*	matrix		)
{
	Matrix*	newMatrix	=	NULL;

	do
	{
		try
		{
			newMatrix	=	new Matrix();
			if( newMatrix == NULL )
			{
				break;
			}
		}
		catch	(	std::bad_alloc&	exception	)
		{
			exception	=	exception;
			break;
		}

		if( newMatrix->allocMatrix(matrix->getCol(), matrix->getRow()) == false )
		{
			delete		newMatrix;
			newMatrix	=	NULL;
			break;
		}

		newMatrix->equal(matrix);

	}while(0);

	return	newMatrix;
}

void		Matrix::destroyMatrix	(	Matrix*	matrix		)
{
	delete		matrix;
}