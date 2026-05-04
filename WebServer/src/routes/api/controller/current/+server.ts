import { fetchGripperCurrent } from '$lib/server/controller';
import { json } from '@sveltejs/kit';
import type { RequestHandler } from './$types';

export const GET: RequestHandler = async ({ fetch }) => {
	try {
		const samples = await fetchGripperCurrent(fetch);
		return json({ samples });
	} catch (error) {
		return json(
			{
				error: error instanceof Error ? error.message : 'Failed to fetch gripper current'
			},
			{ status: 503 }
		);
	}
};
