/*
 * sparse_matrix.cpp
 *
 *  Created on: 2014. 12. 29.
 *      Author: asran
 */

#include "sparse_matrix.h"
#include "matrix_error.h"
#include <math.h>

namespace matrix
{

/**
 * 생성자
 */
SparseMatrix::SparseMatrix			(	void	)
{
}

/**
 * 생성자
 */
SparseMatrix::SparseMatrix			(	size_t		col,	///< 행 크기
											size_t		row		///< 열 크기
										)
{
	allocElems(col, row);
}

/**
 * 복사 생성자
 */
SparseMatrix::SparseMatrix			(	const SparseMatrix&		matrix		///< 복사 될 객체
										)
{
	allocElems(matrix.getCol(), matrix.getRow());
	copyElems(matrix);
}

/**
 * 소멸자
 */
SparseMatrix::~SparseMatrix			(	void	)
{
	freeElems();
}

/**
 * 행렬 요소 값 참조
 * @return		참조한 행렬 요소 값
 */
elem_t		SparseMatrix::getElem		(	size_t		col,	///< 참조 할 행 위치
												size_t		row		///< 참조 할 열 위치
											) const
{
	chkBound(col, row);

	elem_t		value	=	0;

	try
	{
		value	=	mData[col].at(row);
	}
	catch( std::out_of_range&	exception	)
	{
		value	=	0;
	}

	return	value;
}

/**
 * 행렬 요소 값 설정
 */
void		SparseMatrix::setElem		(	size_t		col,	///< 설정 할 행 위치
												size_t		row,	///< 설정 할 열 위치
												elem_t		elem	///< 설정 할 요소 값
											)
{
	chkBound(col, row);

	mData[col][row]	=	elem;
}

/**
 * 행렬 덧셈
 * @return		행렬 덧셈 결과
 */
SparseMatrix	SparseMatrix::add		(	const SparseMatrix&	operand	///< 피연산자
										) const
{
	chkSameSize(operand);

	SparseMatrix	result		=	SparseMatrix(getCol(), getRow());

	for(size_t col=0;col<getCol();col++)
	{
		for(elem_node_itor itor=mData[col].begin();itor!=mData[col].end();itor++)
		{
			result.setElem	(	col,
									itor->first,
									itor->second
								);
		}
	}

	for(size_t col=0;col<getCol();col++)
	{
		for(elem_node_itor itor=operand.mData[col].begin();itor!=operand.mData[col].end();itor++)
		{
			result.setElem	(	col,
									itor->first,
									result.getElem(col, itor->first) + itor->second
								);
		}
	}

	return	result;
}

/**
 * 행렬 뺄셈
 * @return		행렬 뺄셈 결과
 */
SparseMatrix	SparseMatrix::sub		(	const SparseMatrix&	operand	///< 피연산자
										) const
{
	chkSameSize(operand);

	SparseMatrix	result		=	SparseMatrix(getCol(), getRow());

	for(size_t col=0;col<getCol();col++)
	{
		for(elem_node_itor itor=mData[col].begin();itor!=mData[col].end();itor++)
		{
			result.setElem	(	col,
									itor->first,
									itor->second
								);
		}
	}

	for(size_t col=0;col<operand.getCol();col++)
	{
		for(elem_node_itor itor=operand.mData[col].begin();itor!=operand.mData[col].end();itor++)
		{
			result.setElem	(	col,
									itor->first,
									result.getElem(col, itor->first) - itor->second
								);
		}
	}

	return	result;
}

/**
 * 행렬 곱셈
 * @return		행렬 곱셈 결과
 */
SparseMatrix	SparseMatrix::multiply	(	const SparseMatrix&	operand	///< 피연산자
											) const
{
	if( ( getCol() != operand.getRow() ) &&
		( getRow() != operand.getCol() ) )
	{
		throw	matrix::ErrMsg::createErrMsg("행렬 크기가 올바르지 않습니다.");
	}

	SparseMatrix	result	=	SparseMatrix(getCol(), operand.getRow());

	for(size_t col=0;col<getCol();col++)
	{
		for(elem_node_itor itor=mData[col].begin();itor!=mData[col].end();itor++)
		{
			for(elem_node_itor itor2=operand.mData[itor->first].begin();itor2!=operand.mData[itor->first].end();itor2++)
			{
				result.setElem	(	col,
										itor2->first,
										result.getElem(col, itor2->first) + (itor->second * itor2->second)
									);
			}
		}
	}

	return	result;
}

/**
 * 행렬 곱셈
 * @return		행렬 곱셈 결과
 */
SparseMatrix	SparseMatrix::multiply	(	elem_t		operand	///< 피연산자
											) const
{
	SparseMatrix	result	=	SparseMatrix(getCol(), getRow());

	for(size_t col=0;col<getCol();col++)
	{
		for(elem_node_itor itor=mData[col].begin();itor!=mData[col].end();itor++)
		{
			result.setElem	(	col,
									itor->first,
									itor->second * operand
								);
		}
	}

	return	result;
}

/**
 * 전치 행렬 변환 후 곱셈
 * @return		행렬 곱셈 결과
 */
SparseMatrix	SparseMatrix::tmultiply	(	const SparseMatrix&	operand	///< 피연산자
											) const
{
	if( ( getCol() != operand.getCol() ) &&
		( getRow() != operand.getRow() ) )
	{
		throw	matrix::ErrMsg::createErrMsg("행렬 크기가 올바르지 않습니다.");
	}

	SparseMatrix	result	=	SparseMatrix(getCol(), operand.getRow());

	for(size_t col=0;col<getCol();col++)
	{
		for(elem_node_itor itor=mData[col].begin();itor!=mData[col].end();itor++)
		{
			for(elem_node_itor itor2=operand.mData[col].begin();itor2!=operand.mData[col].end();itor2++)
			{
				result.setElem	(	itor->first,
										itor2->first,
										result.getElem(itor->first, itor2->first) + (itor->second * itor2->second)
									);
			}
		}
	}

	return	result;
}

/**
 * 행렬 대입
 * @return		대입 할 행렬
 */
const SparseMatrix&		SparseMatrix::equal			(	const SparseMatrix&	operand	///< 피연산자
															)
{
	try
	{
		chkSameSize(operand);
		copyElems(operand);
	}
	catch( ErrMsg*	exception	)
	{
		freeElems();
		allocElems(operand.getCol(), operand.getRow());
		copyElems(operand);
	}

	return	*this;
}

/**
 * 행렬 방정식 해 계산
 * @return		해 계산 결과
 */
SparseMatrix		SparseMatrix::solution		(	const SparseMatrix&	operand	///< 피연산자
													)
{
	SparseMatrix		x			=	SparseMatrix(this->getRow(), operand.getRow());
	SparseMatrix		r			=	operand - ( (*this) * x );
	SparseMatrix		p			=	r;
	SparseMatrix		rSold		=	r.tmultiply(r);
	SparseMatrix		result		=	x;
	elem_t		min			=	1000;
	size_t		num			=	0;
	bool		foundFlag	=	false;

	for(size_t cnt=0;cnt<1000000;cnt++)
	{
		SparseMatrix	ap		=	(*this) * p;
		elem_t			alpha	=	rSold.getElem(0,0) / (p.tmultiply(ap)).getElem(0,0);

		x	=	x + (p * alpha);
		r	=	r - (ap * alpha);

		SparseMatrix	rsNew	=	r.tmultiply(r);

		elem_t		sqrtVal	=	sqrt(rsNew.getElem(0,0));

		if( min > sqrtVal )
		{
			num		=	cnt;
			min		=	sqrtVal;
			result	=	x;
		}

		if( sqrtVal < 0.001 )
		{
			foundFlag	=	true;
			break;
		}

		p		=	r + ( p * (rsNew.getElem(0,0) / rSold.getElem(0,0) ) );
		rSold	=	rsNew;
	}

	printf("min = %3.5f\nnum = %ld\n", min, num);
	if( foundFlag != true )
	{
		x	=	result;
	}

	return	x;
}

/**
 * 행렬 데이터 공간 할당
 * @exception		메모리 할당 실패 시 에러 발생
 */
void		SparseMatrix::allocElems		(	size_t		col,	///< 행 크기
												size_t		row		///< 열 크기
											)
{
	try
	{
		mCol	=	col;
		mRow	=	row;

		mData	=	new elem_node_t[col];
	}
	catch (	std::bad_alloc&	exception		)
	{
		throw matrix::ErrMsg::createErrMsg(exception.what());
	}
}

/**
 * 행렬 데이터 공간 할당 해제
 */
void		SparseMatrix::freeElems		(	void	)
{
	delete[]	mData;
	mCol	=	0;
	mRow	=	0;
}

/**
 * 행렬 데이터 복사
 */
void		SparseMatrix::copyElems		(	const SparseMatrix&		matrix		///< 복사 할 행렬
											)
{
	for(size_t col=0;col<getCol();col++)
	{
		mData[col].clear();
		mData[col]		=	matrix.mData[col];
	}
}

/**
 * 같은 크기의 행렬인지 검사
 * @exception		행렬이 같은 크기가 아닐 경우 예외 발생
 */
void		SparseMatrix::chkSameSize	(	const SparseMatrix&		matrix		///< 비교 할 행렬
											) const
{
	if( ( getCol() != matrix.getCol() ) ||
		( getRow() != matrix.getRow() ) )
	{
		throw matrix::ErrMsg::createErrMsg("행렬 크기가 올바르지 않습니다.");
	}
}

/**
 * 행렬 요소 참조 범위 검사
 * @exception		참조 범위 밖일 경우 예외 발생
 */
void		SparseMatrix::chkBound		(	size_t		col,	///< 참조 할 행 위치
												size_t		row		///< 참조 할 열 위치
											) const
{
	if( ( col >= mCol ) ||
		( row >= mRow ) )
	{
		throw	matrix::ErrMsg::createErrMsg("범위를 넘어서는 참조입니다.");
	}
}

};